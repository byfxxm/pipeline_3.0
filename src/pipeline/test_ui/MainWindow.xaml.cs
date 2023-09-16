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
        delegate void SetGlobalOutputCallback(string v);

        public MainWindow()
        {
            InitializeComponent();
        }

        [DllImport("pipeline_adapter.dll")]
        private static extern IntPtr CreateAuto();

        [DllImport("pipeline_adapter.dll")]
        private static extern void DeleteAuto(IntPtr v);

        [DllImport("pipeline_adapter.dll")]
        private static extern void Start(IntPtr v);

        [DllImport("pipeline_adapter.dll")]
        private static extern void Stop(IntPtr v);

        [DllImport("pipeline_adapter.dll")]
        private static extern void Wait(IntPtr v);

        [DllImport("pipeline_adapter.dll")]
        private static extern void SetGlobalOutput(SetGlobalOutputCallback f);

        private void Button_Click(object sender, RoutedEventArgs e)
        {
                SetGlobalOutput(Input);
                var auto = CreateAuto();
                Start(auto);
                Wait(auto);
                Text1.Text = text1;
        }

        private void Input(string s)
        {
            text1 += s;
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
        }

        private string text1 = new("");
    }
}