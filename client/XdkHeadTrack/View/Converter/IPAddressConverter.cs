using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace XdkHeadTrack.View.Converter
{
	public class IPAddressConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter,
			CultureInfo culture)
		{
			if (targetType != typeof(string))
				throw new InvalidOperationException("The target must be a string");

			if (value == null)
				return IPAddress.Any;

			if (value.GetType() == typeof(IPAddress))
				return ((IPAddress)value).ToString();
			else
				throw new InvalidOperationException("Could not convert the given input value of type: " + value.GetType());
		}

		public object ConvertBack(object value, Type targetType, object parameter,
			CultureInfo culture)
		{
			if (targetType != typeof(IPAddress))
				throw new InvalidOperationException("The target must be an IPAddress");

			if (value == null)
				return IPAddress.Any;

			IPAddress address;
			if (value.GetType() == typeof(string))
			{
				if (string.IsNullOrWhiteSpace((string)value))
					return IPAddress.Any;

				if (IPAddress.TryParse((string)value, out address))
					return address;

				IPHostEntry entry = Dns.GetHostEntry((string)value);
				if (entry.AddressList.Length > 0)
					address = entry.AddressList[0];
				else
					throw new ArgumentException("No IP host entries registered for the given address");
			}
			else
				throw new InvalidOperationException("Could not convert the given input value of type: " + value.GetType());

			return address;
		}
	}
}
