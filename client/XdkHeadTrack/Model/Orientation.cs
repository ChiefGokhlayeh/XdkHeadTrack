using System;
using System.Windows.Media.Media3D;
using XdkHeadTrack.Utils;

namespace XdkHeadTrack.Model
{
	public class Orientation
	{
		public Point3D Translation { get; private set; }
		public Quaternion Rotation { get; private set; }

		private readonly Lazy<byte[]> _raw = new Lazy<byte[]>(() => new byte[6 * sizeof(double)]);
		public byte[] Bytes
		{
			get
			{
				if (_raw.IsValueCreated)
				{
					Vector3D yawPitchRoll = Rotation.QuatToEuler();

					double yaw = yawPitchRoll.X * (360.0D / Math.PI);
					int i = 0;
					Array.Copy(BitConverter.GetBytes(Translation.X), 0, _raw.Value, sizeof(double) * i++, sizeof(double));
					Array.Copy(BitConverter.GetBytes(Translation.Y), 0, _raw.Value, sizeof(double) * i++, sizeof(double));
					Array.Copy(BitConverter.GetBytes(Translation.Z), 0, _raw.Value, sizeof(double) * i++, sizeof(double));
					Array.Copy(BitConverter.GetBytes(yaw), 0, _raw.Value, sizeof(double) * i++, sizeof(double));
					Array.Copy(BitConverter.GetBytes(yawPitchRoll.Y * (180.0D / Math.PI)), 0, _raw.Value, sizeof(double) * i++, sizeof(double));
					Array.Copy(BitConverter.GetBytes(yawPitchRoll.Z * (180.0D / Math.PI)), 0, _raw.Value, sizeof(double) * i++, sizeof(double));
				}
				return _raw.Value;
			}
		}

		public Orientation(Point3D translation, Quaternion rotation)
		{
			Translation = translation;
			Rotation = rotation;
		}
	}
}
