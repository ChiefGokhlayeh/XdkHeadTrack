using System;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using System.Windows.Media.Media3D;
using XdkHeadTrack.Utils;

namespace XdkHeadTrack.Model
{
	public class UdpOrientationSender : BaseSynchronizedNotifyPropertyChanged
	{
		public UdpClient ClientV4 { get; private set; }
		public UdpClient ClientV6 { get; private set; }

		private IPAddress _targetIpAddress;

		public IPAddress TargetIPAddress
		{
			get
			{
				return _targetIpAddress;
			}
			set
			{
				_targetIpAddress = value;
				NotifyPropertyChanged();
				NotifyPropertyChanged("TargetEndpoint");
			}
		}

		private short _targetPort;

		public short TargetPort
		{
			get
			{
				return _targetPort;
			}
			set
			{
				_targetPort = value;
				NotifyPropertyChanged();
				NotifyPropertyChanged("TargetEndpoint");
			}
		}


		public IPEndPoint TargetEndpoint
		{
			get
			{
				if (TargetIPAddress == null)
					return null;
				else
					return new IPEndPoint(TargetIPAddress, TargetPort);
			}
		}

		public UdpOrientationSender()
		{
			ClientV4 = new UdpClient(AddressFamily.InterNetwork);
			ClientV6 = new UdpClient(AddressFamily.InterNetworkV6);
		}

		public void Send(Orientation orientation)
		{
			UdpClient c;
			if (TargetEndpoint.AddressFamily == AddressFamily.InterNetwork)
				c = ClientV4;
			else if (TargetEndpoint.AddressFamily == AddressFamily.InterNetworkV6)
				c = ClientV6;
			else
				throw new InvalidOperationException("IPEndPoint address-familiy not supported, use IPv4 or IPv6.");

			int len = c.Send(orientation.Bytes, orientation.Bytes.Length, TargetEndpoint);
			Debug.Assert(len == orientation.Bytes.Length);
		}

		public async Task SendAsync(Orientation orientation)
		{
			UdpClient c;
			if (TargetEndpoint.AddressFamily == AddressFamily.InterNetwork)
				c = ClientV4;
			else if (TargetEndpoint.AddressFamily == AddressFamily.InterNetworkV6)
				c = ClientV6;
			else
				throw new InvalidOperationException("IPEndPoint address-familiy not supported, use IPv4 or IPv6.");

			int len = await c.SendAsync(orientation.Bytes, orientation.Bytes.Length, TargetEndpoint);
			Debug.Assert(len == orientation.Bytes.Length);
		}

		public void Send(Quaternion rotation)
		{
			Send(new Orientation(new Point3D(), rotation));
		}

		public async Task SendAsync(Quaternion rotation)
		{
			await SendAsync(new Orientation(new Point3D(), rotation));
		}

		public void Send(Point3D translation)
		{
			Send(new Orientation(translation, Quaternion.Identity));
		}

		public async Task SendAsync(Point3D translation)
		{
			await SendAsync(new Orientation(translation, Quaternion.Identity));
		}
	}
}
