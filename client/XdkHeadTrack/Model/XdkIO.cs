using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.IO.Ports;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Media.Media3D;
using XdkHeadTrack.Utils;

namespace XdkHeadTrack.Model
{
	public class XdkIO : BaseSynchronizedNotifyPropertyChanged
	{
		public event EventHandler<XdkIORotationEventArgs> RotationDataReceived;

		private Quaternion _calibratedRotation;
		public Quaternion CalibratedRotation
		{
			get { return _calibratedRotation; }
			private set { _calibratedRotation = value; NotifyPropertyChanged(); }
		}

		private Quaternion _rawRotation;
		public Quaternion RawRotation
		{
			get { return _rawRotation; }
			private set { _rawRotation = value; NotifyPropertyChanged(); }
		}

		private Quaternion _calibrationCorrection;
		public Quaternion CalibrationCorrection
		{
			get { return _calibrationCorrection; }
			private set { _calibrationCorrection = value; NotifyPropertyChanged(); }
		}

		public bool IsSerialConnected
		{
			get { return _port.IsOpen; }
		}

		public string CurrentSerialPortName
		{
			get { lock (_portSyncLock) { return _port.PortName; } }
		}

		private SerialPort _port;
		private readonly object _portSyncLock = new object();

		public XdkIO()
		{
			_port = new SerialPort();
			_port.BaudRate = 115200;
			_port.DataReceived += HandlePortDataReceived;
		}

		public void ConnectSerial(string portName)
		{
			lock (_portSyncLock)
			{
				if (!_port.IsOpen)
				{
					_port.PortName = portName;
					_port.Open();
				}
				else
				{
					throw new InvalidOperationException("Serial already connected to port: " + _port.PortName);
				}
			}
			NotifyPropertyChanged("IsSerialConnected");
		}

		public void DisconnectSerial()
		{
			lock (_portSyncLock)
			{
				if (_port.IsOpen)
					_port.Close();
			}
			NotifyPropertyChanged("IsSerialConnected");
		}

		public async Task DisconnectSerialAsync()
		{
			await Task.Run(() =>
			{
				lock (_portSyncLock)
				{
					if (_port.IsOpen)
						_port.Close();
				}
			});
			NotifyPropertyChanged("IsSerialConnected");
		}

		private void FireRotationDataReceived(XdkIORotationEventArgs args)
		{
			RotationDataReceived?.Invoke(this, args);
		}

		#region Event Handlers
		private void HandlePortDataReceived(object sender, SerialDataReceivedEventArgs e)
		{
			Quaternion? q = null;
			Quaternion? cali = null;
			if (e.EventType == SerialData.Chars)
			{
				lock (_portSyncLock)
				{
					if (_port.IsOpen)
					{
						Stream stream = _port.BaseStream;
						StreamReader streamReader = new StreamReader(stream);
						string line = streamReader.ReadLine();
						Debug.WriteLine(line);
						Regex regex = new Regex(@">>QUAT.*?([0-9\.\-\+]{1,}).*?([0-9\.\-\+]{1,}).*?([0-9\.\-\+]{1,}).*?([0-9\.\-\+]{1,})");
						Match match = regex.Match(line);
						if (match.Groups.Count >= 5)
						{
							IFormatProvider format = CultureInfo.GetCultureInfo("en-US").NumberFormat;
							q = new Quaternion(
								Convert.ToDouble(match.Groups[2].Value, format),
								Convert.ToDouble(match.Groups[3].Value, format),
								Convert.ToDouble(match.Groups[4].Value, format),
								Convert.ToDouble(match.Groups[1].Value, format));
						}
						else
						{
							regex = new Regex(@">>CALI.*?([0-9\.\-\+]{1,}).*?([0-9\.\-\+]{1,}).*?([0-9\.\-\+]{1,}).*?([0-9\.\-\+]{1,})");
							match = regex.Match(line);
							if (match.Groups.Count >= 5)
							{
								IFormatProvider format = CultureInfo.GetCultureInfo("en-US").NumberFormat;
								cali = new Quaternion(
									Convert.ToDouble(match.Groups[2].Value, format),
									Convert.ToDouble(match.Groups[3].Value, format),
									Convert.ToDouble(match.Groups[4].Value, format),
									Convert.ToDouble(match.Groups[1].Value, format));
							}
						}
					}
				}
				if (q != null)
				{
					RawRotation = q.Value;
					Quaternion tmp = CalibrationCorrection;
					tmp.Invert();
					CalibratedRotation = q.Value * tmp * new Quaternion(new Vector3D(0, 0, 1), 180);
					FireRotationDataReceived(new XdkIORotationEventArgs(q.Value));
				}
				if (cali != null)
				{
					CalibrationCorrection = cali.Value;
				}
			}
		}
		#endregion
	}

	public class XdkIORotationEventArgs
	{
		public Quaternion Rotation { get; private set; }

		public XdkIORotationEventArgs(Quaternion rotation)
		{
			Rotation = rotation;
		}
	}
}
