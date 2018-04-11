using System;
using System.Windows.Data;
using System.Windows.Media.Media3D;

namespace XdkHeadTrack.View.Converter
{
	public class NegateVector3DConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter,
			System.Globalization.CultureInfo culture)
		{
			if (targetType != typeof(Vector3D))
				throw new InvalidOperationException("The target must be a Vector3D");

			Vector3D v;
			if (value.GetType() == typeof(Vector3D))
				v = ((Vector3D)value);
			else if (value.GetType() == typeof(Point3D))
				v = (Vector3D)((Point3D)value);
			else if (value.GetType() == typeof(Tuple<double, double, double>))
				v = new Vector3D(((Tuple<double, double, double>)value).Item1,
					((Tuple<double, double, double>)value).Item2,
					((Tuple<double, double, double>)value).Item3);
			else if (value.GetType() == typeof(TranslateTransform3D))
				v = new Vector3D(((TranslateTransform3D)value).OffsetX,
					((TranslateTransform3D)value).OffsetY,
					((TranslateTransform3D)value).OffsetZ);
			else
				throw new InvalidOperationException("Could not convert the given input value of type: " + value.GetType());
			v.Negate();
			return v;
		}

		public object ConvertBack(object value, Type targetType, object parameter,
			System.Globalization.CultureInfo culture)
		{
			throw new NotSupportedException();
		}
	}
}
