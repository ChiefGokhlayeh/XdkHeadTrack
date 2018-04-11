using System;
using System.Collections.Generic;
using System.Linq;
using System.Diagnostics.Contracts;
using System.IO.Ports;
using System.Management;

namespace XdkHeadTrack.Utils
{
	public class SerialPortService
	{
		private readonly static Lazy<SerialPortService> _instance = new Lazy<SerialPortService>(() => new SerialPortService());
		public static SerialPortService Instance => _instance.Value;

		private readonly object _lock = new object();
		private string[] _previousSerialPorts;
		private ManagementEventWatcher _arrival;
		private ManagementEventWatcher _removal;

		public event EventHandler<PortsChangedArgs> PortsChanged;

		private SerialPortService()
		{
			_previousSerialPorts = GetPortNames();

			WqlEventQuery deviceArrivalQuery = new WqlEventQuery("SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2");
			WqlEventQuery deviceRemovalQuery = new WqlEventQuery("SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 3");

			_arrival = new ManagementEventWatcher(deviceArrivalQuery);
			_removal = new ManagementEventWatcher(deviceRemovalQuery);

			_arrival.EventArrived += (o, args) => RaisePortsChangedIfNecessary(SerialPortServiceEventType.Insertion);
			_removal.EventArrived += (o, args) => RaisePortsChangedIfNecessary(SerialPortServiceEventType.Removal);
		}

		~SerialPortService()
		{
			StopMonitoring();
			_arrival.Dispose();
			_removal.Dispose();
		}

		public void StopMonitoring()
		{
			lock (_lock)
			{
				_arrival.Stop();
				_removal.Stop();
			}
		}

		public void StartMonitoring()
		{
			lock (_lock)
			{
				_arrival.Start();
				_removal.Start();
			}
		}

		private void RaisePortsChangedIfNecessary(SerialPortServiceEventType eventType)
		{
			lock (_lock)
			{
				string[] availableSerialPorts = GetPortNames();
				if (!_previousSerialPorts.SequenceEqual(availableSerialPorts))
				{
					_previousSerialPorts = availableSerialPorts;
					PortsChanged?.Invoke(this, new PortsChangedArgs(eventType, _previousSerialPorts));
				}
			}
		}

		public string[] GetPortNames() => SerialPort.GetPortNames();
	}

	public enum SerialPortServiceEventType
	{
		Insertion,
		Removal,
	}

	public class PortsChangedArgs : EventArgs
	{
		private readonly SerialPortServiceEventType _eventType;
		private readonly string[] _serialPorts;

		public string[] SerialPorts => _serialPorts;
		public SerialPortServiceEventType EventType => _eventType;

		public PortsChangedArgs(SerialPortServiceEventType eventType, string[] serialPorts)
		{
			_eventType = eventType;
			_serialPorts = serialPorts;
		}
	}
}
