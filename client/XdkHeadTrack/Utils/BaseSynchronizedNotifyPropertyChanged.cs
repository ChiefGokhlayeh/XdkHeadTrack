using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Threading;

namespace XdkHeadTrack.Utils
{
	public class BaseSynchronizedNotifyPropertyChanged : BaseNotifyPropertyChanged
	{
		protected bool IsSynchronized => Thread.CurrentThread == _dispatcher.Thread;
		protected readonly Dispatcher _dispatcher;

		public BaseSynchronizedNotifyPropertyChanged() : this(Dispatcher.CurrentDispatcher) { }

		public BaseSynchronizedNotifyPropertyChanged(Dispatcher dispatcher)
		{
			_dispatcher = dispatcher;
		}

		protected override void NotifyPropertyChanged([CallerMemberName] String propertyName = "")
		{
			if (IsSynchronized)
				base.NotifyPropertyChanged(propertyName);
			else
				_dispatcher.BeginInvoke(new Action(() => base.NotifyPropertyChanged(propertyName)), null);
		}
	}
}
