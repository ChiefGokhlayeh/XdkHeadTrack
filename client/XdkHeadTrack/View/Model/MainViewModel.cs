using HelixToolkit.Wpf;
using System;
using System.IO;
using System.IO.Ports;
using System.Net;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Windows.Media.Media3D;
using System.Windows.Threading;
using Trustsoft.Commands;
using XdkHeadTrack.Model;
using XdkHeadTrack.Utils;

namespace XdkHeadTrack.View.Model
{
	public class MainViewModel : BaseViewModel
	{
		private Model3D _testModel;
		public Model3D TestModel
		{
			get { return _testModel; }
			private set { _testModel = value; NotifyPropertyChanged(); }
		}

		private bool _isPortConnected;
		public bool IsPortConnected
		{
			get { return _isPortConnected; }
			private set { _isPortConnected = value; NotifyPropertyChanged(); }
		}

		private string[] _serialPortNames;
		public string[] SerialPortNames
		{
			get { return _serialPortNames; }
			private set { _serialPortNames = value; NotifyPropertyChanged(); }
		}

		public string _serialPortNameToUse;
		public string SerialPortNameToUse
		{
			get { return _serialPortNameToUse; }
			set { _serialPortNameToUse = value; NotifyPropertyChanged(); }
		}

		public bool IsUdpSenderActive
		{
			get
			{
				return _udpTask != null && !_udpTask.IsCompleted && !_udpTask.IsFaulted && !_udpTask.IsCanceled;
			}
		}

		public XdkIO Xdk
		{
			get;
			private set;
		}

		public UdpOrientationSender UdpSender
		{
			get;
			private set;
		}

		public ICommand ClosingCommand
		{
			get;
			private set;
		}

		public ICommand ToggleConnectSerialCommand
		{
			get;
			private set;
		}

		public ICommand RefreshPortsCommand
		{
			get;
			private set;
		}

		public ICommand ToggleConnectUdpCommand
		{
			get;
			private set;
		}

		private Task _toggleSerialTask;
		private Task _udpTask;
		private CancellationTokenSource _udpTaskCancelSource;

		public MainViewModel() : this(Dispatcher.CurrentDispatcher) { }

		public MainViewModel(Dispatcher dispatcher) : base(dispatcher)
		{
			Xdk = new XdkIO();
			UdpSender = new UdpOrientationSender();
			UdpSender.TargetIPAddress = IPAddress.Loopback;
			UdpSender.TargetPort = 4242;

			SerialPortNameToUse = Xdk.CurrentSerialPortName;

			SerialPortService.Instance.PortsChanged += OnPortsChanged;
			SerialPortService.Instance.StartMonitoring();

			ClosingCommand = CommandFactory.Create(ClosingCommandExecute);
			ToggleConnectSerialCommand = CommandFactory.Create(ToggleSerialConnectExecuteAsync, CanToggleSerialConnectExecute, true);
			RefreshPortsCommand = CommandFactory.Create(() => RefreshSerialPorts(), () => !IsPortConnected, true);
			ToggleConnectUdpCommand = CommandFactory.Create(ToggleUdpConnectionExecuteAsync, () => UdpSender.TargetEndpoint != null && 
			(_udpTaskCancelSource == null || !_udpTaskCancelSource.IsCancellationRequested), true);

			RefreshSerialPorts();

			ObjReader objReader = new ObjReader();
			using (Stream stream = new MemoryStream(Properties.Resources.Head))
			{
				TestModel = objReader.Read(stream);
			}
		}

		private void ConnectSerial(string portName)
		{
			Xdk.ConnectSerial(portName);
			Invoke(() =>
			{
				IsPortConnected = Xdk.IsSerialConnected;
			});
			CommandManager.InvalidateRequerySuggested();
		}

		private void DisconnectSerial()
		{
			try
			{
				Xdk.DisconnectSerial();
			}
			finally
			{
				Invoke(() =>
				{
					IsPortConnected = Xdk.IsSerialConnected;
				});
			}
			CommandManager.InvalidateRequerySuggested();
		}

		private async Task DisconnectSerialAsync()
		{
			try
			{
				await Xdk.DisconnectSerialAsync();
			}
			finally
			{
				Invoke(() =>
				{
					IsPortConnected = Xdk.IsSerialConnected;
				});
			}
			CommandManager.InvalidateRequerySuggested();
		}

		private void RefreshSerialPorts()
		{
			string current = Xdk.CurrentSerialPortName;
			SerialPortNames = SerialPort.GetPortNames();
			if (Array.IndexOf(SerialPortNames, SerialPortNameToUse) > -1 && SerialPortNames.Length > 0)
			{
				SerialPortNameToUse = SerialPortNames[0];
			}
			else
			{
				SerialPortNameToUse = null;
			}
			DisconnectSerial();
			CommandManager.InvalidateRequerySuggested();
		}

		private void StartUdpSender()
		{
			_udpTaskCancelSource = new CancellationTokenSource();
			CancellationToken ct = _udpTaskCancelSource.Token;
			_udpTask = Task.Run(() =>
			{
				AutoResetEvent eventSignal = new AutoResetEvent(false);
				EventHandler<XdkIORotationEventArgs> handler = (o, p) =>
				{
					eventSignal.Set();
				};

				try
				{
					Xdk.RotationDataReceived += handler;
					while (!ct.IsCancellationRequested)
					{
						int source = WaitHandle.WaitAny(new[] { ct.WaitHandle, eventSignal });
						if (source == WaitHandle.WaitTimeout)
							continue;
						else if (source == 0)
							break;

						UdpSender.Send(Xdk.CalibratedRotation);
					}
				}
				finally
				{
					Xdk.RotationDataReceived -= handler;
				}
			}, ct);
		}

		private void StopUdpSender()
		{
			if (_udpTask != null)
			{
				_udpTaskCancelSource?.Cancel();
				CommandManager.InvalidateRequerySuggested();
				_udpTask.Wait();
				_udpTaskCancelSource?.Dispose();
				_udpTaskCancelSource = null;
				_udpTask = null;
			}
			CommandManager.InvalidateRequerySuggested();
		}

		private async Task StopUdpSenderAsync()
		{
			if (_udpTask != null)
			{
				_udpTaskCancelSource?.Cancel();
				CommandManager.InvalidateRequerySuggested();
				await _udpTask;
				_udpTaskCancelSource?.Dispose();
				_udpTaskCancelSource = null;
				_udpTask = null;
			}
			CommandManager.InvalidateRequerySuggested();
		}

		#region Command Execute
		private async void ToggleSerialConnectExecuteAsync()
		{
			string portName = SerialPortNameToUse;
			if (IsPortConnected)
				_toggleSerialTask = Task.Run(() => { DisconnectSerial(); });
			else
				_toggleSerialTask = Task.Run(() => { ConnectSerial(portName); });
			await _toggleSerialTask;
			_toggleSerialTask = null;
			CommandManager.InvalidateRequerySuggested();
		}

		private async void ClosingCommandExecute()
		{
			try
			{
				SerialPortService.Instance.StopMonitoring();
			}
			finally
			{
				StopUdpSender();
				await DisconnectSerialAsync();
			}
		}

		private async void ToggleUdpConnectionExecuteAsync()
		{
			if (IsUdpSenderActive)
			{
				await StopUdpSenderAsync();
			}
			else
			{
				StartUdpSender();
			}
		}
		#endregion

		#region Command Can-Execute
		private bool CanToggleSerialConnectExecute()
		{
			return SerialPortNameToUse != null && (_toggleSerialTask == null || _toggleSerialTask.IsCompleted || _toggleSerialTask.IsCanceled || _toggleSerialTask.IsFaulted);
		}
		#endregion

		#region Event Handlers
		private void OnPortsChanged(object sender, PortsChangedArgs e)
		{
			switch (e.EventType)
			{
				case SerialPortServiceEventType.Insertion:
				case SerialPortServiceEventType.Removal:
					Invoke(() => RefreshSerialPorts());
					break;
				default:
					throw new InvalidOperationException();
			}
		}
		#endregion
	}
}
