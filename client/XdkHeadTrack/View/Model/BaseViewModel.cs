using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Threading;
using XdkHeadTrack.Utils;

namespace XdkHeadTrack.View.Model
{
	public abstract class BaseViewModel : BaseSynchronizedNotifyPropertyChanged
	{
		public BaseViewModel() : base()
		{
		}

		public BaseViewModel(Dispatcher dispatcher) : base(dispatcher)
		{
		}

		protected void Invoke(Action callback)
		{
			if (IsSynchronized)
				callback();
			else
				_dispatcher.Invoke(callback);
		}

		protected TResult Invoke<TResult>(Func<TResult> callback)
		{
			if (IsSynchronized)
				return callback();
			else
				return _dispatcher.Invoke<TResult>(callback);
		}


		protected async Task InvokeAsync(Action callback)
		{
			if (IsSynchronized)
				callback();
			else
				await _dispatcher.InvokeAsync(callback);
		}

		protected async Task<TResult> InvokeAsync<TResult>(Func<TResult> callback)
		{
			if (IsSynchronized)
				return callback();
			else
			{
				return await _dispatcher.InvokeAsync(callback);
			}
		}
	}
}
