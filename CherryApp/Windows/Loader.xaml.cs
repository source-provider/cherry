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
using System.Net;
using System.Net.NetworkInformation;
using CherryApp.Classes;
using System.IO;

namespace CherryApp
{
    /// <summary>
    /// interaction logic for MainWindow.xaml
    /// </summary>
    public partial class Loader : Window
    {
        public Loader() => InitializeComponent();
        

        private async void Grid_Loaded(object sender, RoutedEventArgs e)
        {
            if (!NetworkInterface.GetIsNetworkAvailable())
            {
                LoadingData.Content = "Network Not Available! (Please restart when available)";
                return;
            }

            LoadingData.Content = "Checking Version";
            LoadingProgress.Value = 15;
            
            await Task.Delay(1000);

            /* version check */
            string WebVersion = LoaderInformation.VersionLink.Get();
            if (!WebVersion.Contains(LoaderInformation.Version))
            {
                LoadingData.Content = "Incorrect Version! (Download New Version!)";
                return;
            }

            /* folder check */
            LoadingData.Content = $"Checking Folders (0/{LoaderInformation.Folders.Length})";
            double idx = 1;
            foreach(string FolderName in LoaderInformation.Folders)
            {
                LoadingData.Content = $"Checking Folders ({idx++}/{LoaderInformation.Folders.Length})";
                Directory.CreateDirectory($"{AppContext.BaseDirectory}\\{FolderName}");
                await Task.Delay(100);
            }

            LoadingProgress.Value = 50;

            /* file check */
            LoadingData.Content = $"Checking For Updates (May Take A While)";
            await GlobalClass.CAPI.InitializeModule();

            LoadingData.Content = $"Checking Whitelist"; /* no whitelist just yet */
            LoadingProgress.Value = 75;


            LoadingData.Content = $"Finished!";
            LoadingProgress.Value = 100;
            await Task.Delay(1250);

            this.Hide();
            Windows.Main MainWindow = new Windows.Main();
            MainWindow.Show();

            LoadingData.Content = $"Executor should've loaded already";
        }

        private void Window_MouseLeftButtonDown(object sender, MouseButtonEventArgs e) => this.DragMove();
    }
}
