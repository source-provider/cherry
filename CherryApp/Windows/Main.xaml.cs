using CherryApp.Classes;
using ICSharpCode.AvalonEdit.Highlighting.Xshd;
using ICSharpCode.AvalonEdit.Highlighting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Xml;
using System.Diagnostics;
using System.IO;
using Microsoft.Win32;
using System.Reflection;
using System.Runtime.InteropServices;
using static CherryApp.Classes.CherryAPI;
using static CherryApp.Classes.Miscellaneous.Animations;
using static System.Resources.ResXFileRef;
//using System.Windows.Forms;

namespace CherryApp.Windows
{
    /// <summary>
    /// Interaction logic for TheZaaIsUnreal.xaml
    /// </summary>
    public partial class Main : Window
    {
        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool GetCursorPos(ref Win32Point pt);

        [StructLayout(LayoutKind.Sequential)]
        internal struct Win32Point
        {
            public Int32 X;
            public Int32 Y;
        };
        public static Point GetMousePosition()
        {
            var w32Mouse = new Win32Point();
            GetCursorPos(ref w32Mouse);

            return new Point(w32Mouse.X, w32Mouse.Y);
        }


        public Main() => InitializeComponent();
        SaveFileDialog FileDialog = new SaveFileDialog();
        OpenFileDialog opendialogfile = new OpenFileDialog();
        DateTime fsLastRaised;
        private string current_page = "HOMEPAGEBTN";

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Process[] Procs = Process.GetProcessesByName("CherryApp");

            if (Procs.Length > 1)
            {
                MessageBoxResult result = MessageBox.Show("Would you like to kill the other cherry processes?", "Cherry", MessageBoxButton.YesNo, MessageBoxImage.Question);
                if (result == MessageBoxResult.Yes)
                {
                    foreach (Process proc in Procs)
                        if (proc.Id != Process.GetCurrentProcess().Id)
                            proc.Kill();
                }
            }

            using (var Client = new WebClient { Proxy = null })
                using (var XmlTextReader = new XmlTextReader(Client.OpenRead("https://raw.githubusercontent.com/ImmuneLion318/Editor-Syntax/Entry/Syntax/Dark.xshd")))
                    Editor.SyntaxHighlighting = HighlightingLoader.Load(XmlTextReader, HighlightingManager.Instance);

            Editor.Text = "print(\"Hello World!\")";

            foreach (string FilePath in Directory.EnumerateFiles($"{AppContext.BaseDirectory}\\scripts"))
            {
                var Extention = System.IO.Path.GetExtension(FilePath);
                string FileName = FilePath.Replace($"{AppContext.BaseDirectory}\\scripts\\", "");

                if (Extention == ".lua" || Extention == ".txt")
                    ScriptList.Items.Add(FileName);
            }

            ScriptList.SelectionChanged += (s, e) =>
            {
                string FilePath = $"{AppContext.BaseDirectory}\\scripts\\{ScriptList.SelectedItem}";
                if (File.Exists(FilePath))
                    Editor.Text = File.ReadAllText(FilePath);
            };

            fsLastRaised = DateTime.Now;

            FileSystemWatcher fs = new FileSystemWatcher($"{AppContext.BaseDirectory}\\scripts\\", "*.*");
            fs.EnableRaisingEvents = true;

            fs.Created += (s, e) =>
            {
                try
                {
                    if (DateTime.Now.Subtract(fsLastRaised).TotalMilliseconds > 1000)
                    {
                        string CreatedFileName = e.Name;
                        FileInfo CreatedFile = new FileInfo(CreatedFileName);
                        string Extention = CreatedFile.Extension;
                        string eventoccured = e.ChangeType.ToString();

                        fsLastRaised = DateTime.Now;

                        if (Extention == ".lua" || Extention == ".txt")
                        {
                            string FileName = CreatedFile.FullName.Replace($"{AppContext.BaseDirectory}\\scripts\\", "");
                            ScriptList.Items.Add(FileName);
                        }

                    }
                } catch { }
            };

            fs.Renamed += (s, e) =>
            {
                string oldName = e.OldFullPath.Replace($"{AppContext.BaseDirectory}\\scripts\\", "");
                string newName = e.FullPath.Replace($"{AppContext.BaseDirectory}\\scripts\\", "");

                try
                {
                    if (DateTime.Now.Subtract(fsLastRaised).TotalMilliseconds > 1000)
                    {
                        fsLastRaised = DateTime.Now;
                        System.Threading.Thread.Sleep(100);
                        int idx = ScriptList.Items.IndexOf(oldName);
                        ScriptList.Items.RemoveAt(idx);
                        ScriptList.Items.Insert(idx, newName);
                    }
                } catch (Exception ex)
                {

                }
            };

            fs.Deleted += (s, e) =>
            {
                string newName = e.FullPath.Replace($"{AppContext.BaseDirectory}\\scripts\\", "");

                try
                {
                    if (DateTime.Now.Subtract(fsLastRaised).TotalMilliseconds > 1000)
                    {
                        fsLastRaised = DateTime.Now;
                        System.Threading.Thread.Sleep(100);
                        ScriptList.Items.Remove(newName);
                    }
                }
                catch (Exception ex)
                {}
            };


            switch (current_page)
            {
                case "HOMEPAGEBTN":
                    TweenContentToColor(HomePageBtn, ChoosenColor);
                    break;
                case "EXECUTORPAGEBTN":
                    TweenContentToColor(ExecutorPageBtn, ChoosenColor);
                    break;
                case "SCRIPTHUBPAGEBTN":
                    TweenContentToColor(ScriptHubPageBtn, ChoosenColor);
                    break;
                case "OUTPUTPAGEBTN":
                    TweenContentToColor(OutputPageBtn, ChoosenColor);
                    break;
                case "SETTINGSPAGEBTN":
                    TweenContentToColor(SettingsPageBtn, ChoosenColor);
                    break;
            }
        }

        private async void Window_MouseDown(object sender, MouseButtonEventArgs e)
        {
            try
            {
                if (e.ChangedButton == MouseButton.Left && e.ButtonState == MouseButtonState.Pressed)
                {
                    if (WindowState == WindowState.Maximized)
                    {
                        WindowState = WindowState.Normal;
                        Point location = GetMousePosition();
                        this.Left = location.X - (this.Width / 2);
                        this.Top = location.Y;

                        await Task.Delay(100);
                    }

                    this.DragMove();
                }
            } catch { }
        }

        Color BaseColor = Color.FromRgb(96, 96, 96);
        Color ChoosenColor = Color.FromRgb(186, 186, 186);
        private void pick_color(string page_name)
        {
            if (current_page == page_name)
                return;

            TweenContentToColor(HomePageBtn, BaseColor);
            TweenContentToColor(ExecutorPageBtn, BaseColor);
            TweenContentToColor(ScriptHubPageBtn, BaseColor);
            TweenContentToColor(OutputPageBtn, BaseColor);
            TweenContentToColor(SettingsPageBtn, BaseColor);

            switch (page_name)
            {
                case "HOMEPAGEBTN":
                    TweenContentToColor(HomePageBtn, ChoosenColor);
                    break;
                case "EXECUTORPAGEBTN":
                    TweenContentToColor(ExecutorPageBtn, ChoosenColor);
                    break;
                case "SCRIPTHUBPAGEBTN":
                    TweenContentToColor(ScriptHubPageBtn, ChoosenColor);
                    break;
                case "OUTPUTPAGEBTN":
                    TweenContentToColor(OutputPageBtn, ChoosenColor);
                    break;
                case "SETTINGSPAGEBTN":
                    TweenContentToColor(SettingsPageBtn, ChoosenColor);
                    break;
            }
        }

        private void switch_page(object sender, MouseEventArgs e)
        {
            string page_name = (sender as Label).Name.ToUpper();

            Console.WriteLine(page_name);

            if (current_page == page_name)
                return;
            
            pick_color(page_name);
            current_page = page_name;

            switch (page_name)
            {
                case "HOMEPAGEBTN":
                    
                    break;
                case "EXECUTORPAGEBTN":
                    
                    break;
                case "SCRIPTHUBPAGEBTN":
                    
                    break;
                case "OUTPUTPAGEBTN":
                    
                    break;
                case "SETTINGSPAGEBTN":
                    
                    break;
            }
        }

        private void mouseenter_handler(object sender, MouseEventArgs e) {
            switch ((sender as Label).Name.ToUpper())
            {
                case "HOMEPAGEBTN":
                    TweenContentToColor(HomePageBtn, ChoosenColor);
                    break;
                case "EXECUTORPAGEBTN":
                    TweenContentToColor(ExecutorPageBtn, ChoosenColor);
                    break;
                case "SCRIPTHUBPAGEBTN":
                    TweenContentToColor(ScriptHubPageBtn, ChoosenColor);
                    break;
                case "OUTPUTPAGEBTN":
                    TweenContentToColor(OutputPageBtn, ChoosenColor);
                    break;
                case "SETTINGSPAGEBTN":
                    TweenContentToColor(SettingsPageBtn, ChoosenColor);
                    break;
            }
        }
        private void mouseleave_handler(object sender, MouseEventArgs e)
        {
            string page_name = (sender as Label).Name.ToUpper();
            if (current_page == page_name)
                return;

            switch (page_name)
            {
                case "HOMEPAGEBTN":
                    TweenContentToColor(HomePageBtn, BaseColor);
                    break;
                case "EXECUTORPAGEBTN":
                    TweenContentToColor(ExecutorPageBtn, BaseColor);
                    break;
                case "SCRIPTHUBPAGEBTN":
                    TweenContentToColor(ScriptHubPageBtn, BaseColor);
                    break;
                case "OUTPUTPAGEBTN":
                    TweenContentToColor(OutputPageBtn, BaseColor);
                    break;
                case "SETTINGSPAGEBTN":
                    TweenContentToColor(SettingsPageBtn, BaseColor);
                    break;
            }
        }

        /* top bar */
        private void CloseButton_MouseDown(object sender, MouseButtonEventArgs e) => Process.GetCurrentProcess().Kill();
        private void MinimizeButton_MouseDown(object sender, MouseButtonEventArgs e) => WindowState = WindowState.Minimized;

        /* side bar */


        /* bottom */
        private async void ExecuteButton_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (Process.GetProcessesByName("Windows10Universal").Length == 0)
            {
                MessageBox.Show("Roblox not found!", "Cherry", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            await GlobalClass.CAPI.ExecuteScript(Editor.Text);
        }

        private void ClearButton_MouseDown(object sender, MouseButtonEventArgs e) => Editor.Text = "";

        private void OpenButton_MouseDown(object sender, MouseButtonEventArgs e)
        {
            opendialogfile.Filter = "Lua File (*.lua)|*.lua|Text File (*.txt)|*.txt";
            opendialogfile.FilterIndex = 2;
            opendialogfile.RestoreDirectory = true;

            if (opendialogfile.ShowDialog() != true)
                return;

            try
            {
                Editor.Text = "";
                Stream stream;

                if ((stream = opendialogfile.OpenFile()) == null)
                    return;

                using (stream)
                    Editor.Text = File.ReadAllText(opendialogfile.FileName);
            }
            catch (Exception)
            {
               MessageBox.Show("An error occured", "Cherry", MessageBoxButton.OK);
            }
        }

        private void SaveButton_MouseDown(object sender, MouseButtonEventArgs e)
        {
            FileDialog.Filter = "Txt Files (*.txt)|*.txt|Lua Files (*.lua)|*.lua|All Files (*.*)|*.*";
            if (FileDialog.ShowDialog() == true)
            {
                Stream s = FileDialog.OpenFile();
                StreamWriter sw = new StreamWriter(s);
                sw.Write(Editor.Text);
                sw.Close();
            }
        }

        private void InjectButton_MouseDown(object sender, MouseButtonEventArgs e)
        {
            var status = GlobalClass.CAPI.Inject();
            if (status != CherryAPI.InjectionStatus.Success)
            {
                string err = Enum.GetName(typeof(InjectionStatus), status);
                MessageBox.Show($"Error: {err}", "Cherry", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        
    }
}
