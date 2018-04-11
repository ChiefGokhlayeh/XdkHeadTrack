using System.Windows;
using XdkHeadTrack.View.Model;

namespace XdkHeadTrack.View
{
	/// <summary>
	/// Interaktionslogik für MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		private MainViewModel MainViewModel
		{
			get { return (MainViewModel)DataContext; }
		}

		public MainWindow()
		{
			InitializeComponent();
		}
	}
}
