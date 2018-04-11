using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Media3D;
using XdkRotation.Utils;

namespace XdkRotation.Model
{
	public class UdpRotationSender : BaseSynchronizedNotifyPropertyChanged
	{
		public UdpClient Client { get; private set; }

		private IPEndPoint _targetEndpoint;

		public IPEndPoint TargetEndpoint
		{
			get
			{
				return _targetEndpoint;
			}
			set
			{
				_targetEndpoint = value;
				NotifyPropertyChanged();
			}
		}

		public UdpRotationSender()
		{
			Client = new UdpClient();
		}

		public void Send(Orientation orientation)
		{
			int len = Client.Send(orientation.Bytes, orientation.Bytes.Length, TargetEndpoint);
			Debug.Assert(len == orientation.Bytes.Length);
		}

		public async Task SendAsync(Orientation orientation)
		{
			int len = await Client.SendAsync(orientation.Bytes, orientation.Bytes.Length, TargetEndpoint);
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
