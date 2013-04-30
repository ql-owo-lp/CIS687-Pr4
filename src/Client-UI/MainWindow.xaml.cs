/////////////////////////////////////////////////////////////////////
// MainWindow.xaml.cs - Demo of techniques to build WPF interface  //
//                                                                 //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2013       //
/////////////////////////////////////////////////////////////////////
/*
 * Note: 
 *   You must build the MockChannel DLL before building this
 *   WPF application.  If you don't the build will fail.
 *   To do that you simply right-click on the MockChannel project
 *   and select Rebuild.
 */
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.IO;
using System.Threading;
using MahApps.Metro.Controls;
using System.ComponentModel;
using System.Collections.ObjectModel;

namespace SoftwareRepo
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : MetroWindow
	{
		private mockChannel.IChannel chan;
		private Action<string> msgDelegate;
		// defines command string
		private const string LOGIN_MSG = "login ";
		private const string CHKIN_MSG = "check-in ";
		private const string CHKIN_ACK_MSG = "check-in-ack ";
		private const string OPENCHKIN_ACK_MSG = "open-check-in-ack ";
		private const string REMO_PACKAGES = "remote-packages\n";

		// define brush of four different colors
		private SolidColorBrush[] COLOR_BRUSHES = {
			new BrushConverter().ConvertFromString("#4a8af4") as SolidColorBrush,
			new BrushConverter().ConvertFromString("#e9503e") as SolidColorBrush,
			new BrushConverter().ConvertFromString("#f5b400") as SolidColorBrush,
			new BrushConverter().ConvertFromString("#3c8f01") as SolidColorBrush
		};
		private int currentBrush = 0;
		// received remote package list
		private List<string> remotePackageList = new List<string>();

		///////////////////////////////////////////////////////////////
		// constructor
		public MainWindow()
		{
			InitializeComponent();
		}

		///////////////////////////////////////////////////////////////
		// controls status bar
		private string StatusBarContent
		{
			set
			{
				StatusText.Text = value;
				// change color to notify user
				BottomStatusBar.Background = COLOR_BRUSHES[(++currentBrush) % 4];
			}
		}

		///////////////////////////////////////////////////////////////
		// process check-in feedback
		private void procChkIn(string msg)
		{
			bool chkInOpen;
			if (!(chkInOpen = msg.StartsWith(OPENCHKIN_ACK_MSG)) && !msg.StartsWith(CHKIN_ACK_MSG)) return;
			if (chkInOpen)
				ConfirmChkInOpen(msg.Substring(5));	// delete the "open-"
			else
			{
				StatusBarContent = "Check-In succeed!!";
				chan.postMessage(msg);
			}
		}

		///////////////////////////////////////////////////////////////
		// check returned package list
		private void procRemotePackageList(string msg)
		{
			if (!msg.StartsWith(REMO_PACKAGES)) return;
			msg = msg.Substring(REMO_PACKAGES.Length);
			remotePackageList.Clear();
			string[] packages = msg.Split('\n');
			foreach (string p in packages)
				remotePackageList.Add(p);
			displayRemotePackageList();
			StatusBarContent = "Repository list is updated.";
		}

		///////////////////////////////////////////////////////////////
		// load remote package to GUI
		private void displayRemotePackageList()
		{
			RemotePackageList.Items.Clear();
			string lower = SearchPackageText.Text.ToLower();
			foreach (string p in remotePackageList)
			{
				if (string.IsNullOrWhiteSpace(SearchPackageText.Text) || p.ToLower().Contains(lower))
					RemotePackageList.Items.Add(p);
			}
			SortDescription sd = new SortDescription();
			sd.Direction = ListSortDirection.Ascending;
			RemotePackageList.Items.SortDescriptions.Add(sd);
		}

		///////////////////////////////////////////////////////////////
		// process checkin echo message
		private void procMsg(string msg)
		{
			const string MSG_PRE = "msg ";
			const string STATUS_MSG_PRE = "statusmsg ";
			if (msg.StartsWith(STATUS_MSG_PRE))
			{
				StatusBarContent = msg.Substring(STATUS_MSG_PRE.Length);
			}
			else if (msg.StartsWith(MSG_PRE))
			{
				MessageBox.Show(this, StatusBarContent = msg.Substring(MSG_PRE.Length), "Server Echo");
			}
		}

		///////////////////////////////////////////////////////////////
		// message processing thread
		private void Run()
		{
			while (true)
			{
				string msg = chan.getMessage();
				Dispatcher.Invoke(msgDelegate, DispatcherPriority.Background, new object[] { msg });
			}
		}

		///////////////////////////////////////////////////////////////
		// load local package list
		private void loadPackageListTo(ListBox listBox, string path)
		{
			List<string> pList = getLocalPackageList(path);
			listBox.Items.Clear();
			foreach (string file in pList)
				listBox.Items.Add(file);
			SortDescription sd = new SortDescription();
			sd.Direction = ListSortDirection.Ascending;
			listBox.Items.SortDescriptions.Add(sd);
		}

		///////////////////////////////////////////////////////////////
		// get local package list
		private List<string> getLocalPackageList(string path)
		{
			SearchOption sOpt = SearchOption.AllDirectories;
			string[] header_f = Directory.GetFiles(path, "*.h", sOpt),
				cpp_f = Directory.GetFiles(path, "*.cpp", sOpt);
			for (int i = 0; i < header_f.Length; i++)
				header_f[i] = System.IO.Path.GetFileNameWithoutExtension(header_f[i]);
			for (int i = 0; i < cpp_f.Length; i++)
				cpp_f[i] = System.IO.Path.GetFileNameWithoutExtension(cpp_f[i]);
			// now merge the package list
			List<string> packageList = new List<string>();
			foreach (string f in header_f)
				if (!packageList.Contains(f)) packageList.Add(f);
			foreach (string f in cpp_f)
				if (!packageList.Contains(f)) packageList.Add(f);
			return packageList;
		}

		///////////////////////////////////////////////////////////////
		// when the main window is loaded
		private void MainWindow_Loaded(object sender, RoutedEventArgs e)
		{
			// load delegates
			msgDelegate = this.procMsg;
			msgDelegate += this.procRemotePackageList;
			msgDelegate += this.procChkIn;
			DependencyDirPath.Text = CurrentDirPath.Text = Directory.GetCurrentDirectory();
			loadPackageListTo(LocalPackageList, CurrentDirPath.Text);
			loadPackageListTo(LocalPackageDependencyList, DependencyDirPath.Text);
			chan = mockChannel.IChannel.CreateChannel();
			Thread clientReceiveThread = new Thread(new ThreadStart(this.Run));
			clientReceiveThread.IsBackground = true;
			clientReceiveThread.Start();
		}

		///////////////////////////////////////////////////////////////
		// show a dialog window to ask user to choose which directory to browse and load package list to list box
		private void browsePackageFromPath(TextBox tb, ListBox lb)
		{
			System.Windows.Forms.FolderBrowserDialog fbd = new System.Windows.Forms.FolderBrowserDialog();
			fbd.ShowNewFolderButton = false;
			fbd.SelectedPath = tb.Text;
			System.Windows.Forms.DialogResult result = fbd.ShowDialog();
			if (result == System.Windows.Forms.DialogResult.OK)
			{
				Directory.SetCurrentDirectory(tb.Text = fbd.SelectedPath);
				loadPackageListTo(lb, tb.Text);
			}
		}

		///////////////////////////////////////////////////////////////
		// browse local package
		private void Button_BrowseLocalPack(object sender, RoutedEventArgs e)
		{
			LocalProgress.IsActive = true;
			browsePackageFromPath(CurrentDirPath, LocalPackageList);
			LocalProgress.IsActive = false;
		}

		///////////////////////////////////////////////////////////////
		// browse local package dependencies
		private void Button_BrowsePackDepen(object sender, RoutedEventArgs e)
		{
			LocalProgress.IsActive = true;
			browsePackageFromPath(DependencyDirPath, LocalPackageDependencyList);
			LocalProgress.IsActive = false;
		}

		///////////////////////////////////////////////////////////////
		// refresh package list of remote repository
		private void RefreshButton_Click(object sender, RoutedEventArgs e)
		{
			// basically, it sends a message, then remote will reply both the namespaces and package names
			chan.postMessage("refresh-repository");
		}

		///////////////////////////////////////////////////////////////
		// extract selected packages
		private void ExtractButton_Click(object sender, RoutedEventArgs e)
		{
			if (RemotePackageList.SelectedItems.Count < 1)
			{
				MessageBox.Show(this, "You must select at least one package to extract!");
				return;
			}
			StringBuilder sb = new StringBuilder();
			foreach (string item in RemotePackageList.SelectedItems)
			{
				sb.Append("\n" + item);
			}
			// get package including all dependencies?
			sb.Insert(0, IsExtractAll.IsChecked.Value ? "all" : "one");
			sb.Insert(0, "extract ");
			chan.postMessage(sb.ToString());
		}

		///////////////////////////////////////////////////////////////
		// do check in
		private void CheckInButton_Click(object sender, RoutedEventArgs e)
		{
			List<string> packageFiles = new List<string>();
			StringBuilder sb = new StringBuilder();
			string item = LocalPackageList.SelectedItem as string;
			if (string.IsNullOrWhiteSpace(NS.Text) || NS.Text.Contains(' '))
			{
				MessageBox.Show(this, "Namespace must not be null or contain any whitespace.");
				return;
			}
			if (string.IsNullOrEmpty(item))
			{
				MessageBox.Show(this, "At least one package must be selected!\nClick on path to choose package directory.");
				return;
			}
			// add dependency list
			foreach (string depen in LocalPackageDependencyList.SelectedItems)
			{
				if (depen != item)
					sb.Append("\n" + depen + " 0");
			}
			sb.Append("\npackage files");
			string[] header_f = Directory.GetFiles(CurrentDirPath.Text, item + ".h", System.IO.SearchOption.AllDirectories),
			cpp_f = Directory.GetFiles(CurrentDirPath.Text, item + ".cpp", System.IO.SearchOption.AllDirectories);
			if (header_f.Length > 1 || cpp_f.Length > 1)
			{
				MessageBox.Show(this, "More than one \".h\" files or \".cpp\" files exist in selected directory for this package!\nOne package should have at most one \".h\" file and one \".cpp\" file!");
				return;
			}
			// append two file path here
			if (header_f.Length > 0)
				sb.Append("\n" + header_f[0]);
			if (cpp_f.Length > 0)
				sb.Append("\n" + cpp_f[0]);
			sb.Insert(0, CHKIN_MSG + item + " " + NS.Text + " " + UnameText.Text + " 0");
			chan.postMessage(sb.ToString());
		}

		///////////////////////////////////////////////////////////////
		// login button
		private void LoginButton_Click(object sender, RoutedEventArgs e)
		{
			if (string.IsNullOrWhiteSpace(UnameText.Text) || string.IsNullOrWhiteSpace(PassWdText.Password)
				|| UnameText.Text.Contains(' ') || PassWdText.Password.Contains(' '))
			{
				MessageBox.Show(this, "You must input username and password to login!");
				return;
			}
			// post a login message
			chan.postMessage(LOGIN_MSG + UnameText.Text + " " + PassWdText.Password);
		}

		///////////////////////////////////////////////////////////////
		// click on logout button
		private void LogoutButton_Click(object sender, RoutedEventArgs e)
		{
			UnameText.IsEnabled = true;
			PassWdText.IsEnabled = true;
			chan.postMessage("logout");
		}

		///////////////////////////////////////////////////////////////
		// instant search
		private void SearchPackageText_PreviewKeyUp(object sender, KeyEventArgs e)
		{
			displayRemotePackageList();
		}

		///////////////////////////////////////////////////////////////
		// a prompt asking user to check-in open
		private void ConfirmChkInOpen(string msg)
		{
			MessageBoxResult prompt = MessageBox.Show(this,
				"Not all of this package's dependencies are ready in remote repository. " +
				"Continue on Checking-in current package will force it be \"check-in open\".\n" +
				"Do you still want to continue?", "Check-In Open!!",
				MessageBoxButton.YesNo, MessageBoxImage.Warning);
			switch (prompt)
			{
				case MessageBoxResult.Yes:
					StatusBarContent = "Check-In succeed!!";
					chan.postMessage(msg);
					break;
				case MessageBoxResult.No:
				default:
					StatusBarContent = "Give up checking-in current package.";
					break;
			}
		}
	}
}
