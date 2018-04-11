using System;
using System.Windows.Media.Media3D;

namespace XdkHeadTrack.Utils
{
	public static class QuaternionExtension
	{
		public static Vector3D ToEulerAngles(this Quaternion q)
		{
			// Store the Euler angles in radians
			Vector3D pitchYawRoll = new Vector3D();

			double sqw = q.W * q.W;
			double sqx = q.X * q.X;
			double sqy = q.Y * q.Y;
			double sqz = q.Z * q.Z;

			// If quaternion is normalised the unit is one, otherwise it is the correction factor
			double unit = sqx + sqy + sqz + sqw;
			double test = q.X * q.Y + q.Z * q.W;

			if (test > 0.5D - double.Epsilon * unit)
			{
				// Singularity at north pole
				pitchYawRoll.Y = 2D * Math.Atan2(q.X, q.W);
				pitchYawRoll.X = Math.PI * 0.5D;
				pitchYawRoll.Z = 0D;
				return pitchYawRoll;
			}
			else if (test < -0.5D - double.Epsilon * unit)
			{
				// Singularity at south pole
				pitchYawRoll.Y = -2D * Math.Atan2(q.X, q.W);
				pitchYawRoll.X = -Math.PI * 0.5D;
				pitchYawRoll.Z = 0D;
				return pitchYawRoll;
			}
			else
			{ 
				pitchYawRoll.Y = Math.Atan2(2D * q.Y * q.W - 2D * q.X * q.Z, sqx - sqy - sqz + sqw);
				pitchYawRoll.X = Math.Asin(2D * test / unit);
				pitchYawRoll.Z = Math.Atan2(2D * q.X * q.W - 2D * q.Y * q.Z, -sqx + sqy - sqz + sqw);
			}

			return pitchYawRoll;
		}

		public static Quaternion CreateFromYawPitchRoll(double yaw, double pitch, double roll)
		{
			double rollOver2 = roll * 0.5f;
			double sinRollOver2 = Math.Sin(rollOver2);
			double cosRollOver2 = Math.Cos(rollOver2);
			double pitchOver2 = pitch * 0.5f;
			double sinPitchOver2 = Math.Sin(pitchOver2);
			double cosPitchOver2 = Math.Cos(pitchOver2);
			double yawOver2 = yaw * 0.5f;
			double sinYawOver2 = Math.Sin(yawOver2);
			double cosYawOver2 = Math.Cos(yawOver2);
			Quaternion result = new Quaternion();
			result.X = cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2;
			result.Y = cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2;
			result.Z = cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2;
			result.W = sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2;
			return result;
		}

		public static Vector3D QuatToEuler(this Quaternion q)
		{
			//This is the code from 
			//http://www.mawsoft.com/blog/?p=197
			var rotation = q;
			double q0 = rotation.W;
			double q1 = rotation.Y;
			double q2 = rotation.X;
			double q3 = rotation.Z;

			Vector3D radAngles = new Vector3D();
			radAngles.Y = Math.Atan2(2 * (q0 * q1 + q2 * q3), 1 - 2 * (Math.Pow(q1, 2) + Math.Pow(q2, 2)));
			radAngles.Z = Math.Asin(2 * (q0 * q2 - q3 * q1));
			radAngles.X = Math.Atan2(2 * (q0 * q3 + q1 * q2), 1 - 2 * (Math.Pow(q2, 2) + Math.Pow(q3, 2)));

			return radAngles;
		}
	}
}
