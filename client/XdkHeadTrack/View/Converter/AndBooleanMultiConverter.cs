using System;
using System.Globalization;
using System.Windows.Data;

namespace XdkHeadTrack.View.Converter
{
	public class AndBooleanMultiConverter : IMultiValueConverter
	{
		public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
		{
			if (targetType != typeof(bool))
				throw new InvalidOperationException("The target must be a boolean");

			return Array.TrueForAll(values, (b) => { return (bool)b; });
		}

		public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
		{
			throw new NotImplementedException();
		}
	}
}
