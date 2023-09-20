using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
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

namespace test_ui
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        delegate void OutputFunc(string v);

        public MainWindow()
        {
            InitializeComponent();
            timer.Tick += Timer_Tick;
            timer.Interval = new TimeSpan(0, 0, 1);
            timer.Start();
        }

        private async void Timer_Tick(object? sender, EventArgs e)
        {
            Text1.Text = await Task.Run(() =>
            {
                var stringbuilder = new StringBuilder();
                SetOutput((string s) =>
                {
                    stringbuilder.Append(s);
                });
                TestMain();
                return stringbuilder.ToString();
            });
        }

        [DllImport("test.dll")]
        private static extern void SetOutput(OutputFunc cb);

        [DllImport("test.dll")]
        private static extern int TestMain();

        private void Button_Click(object sender, RoutedEventArgs e)
        {
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
        }

        private DispatcherTimer timer = new DispatcherTimer();
    }
}