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
        }

        [DllImport("test.dll")]
        private static extern void SetOutput(OutputFunc cb);

        [DllImport("test.dll")]
        private static extern int TestMain();

        private async void Button_Click(object sender, RoutedEventArgs e)
        {
            string text = "";
            await Task.Run(() =>
            {
                SetOutput((string s) =>
                {
                    text += s;
                });
                TestMain();
            });
            Text1.Text = text;
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
        }
    }
}