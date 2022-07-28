// ----------------------------------------------------------------------------------------------
// Copyright (c) Mattias Högström.
// ----------------------------------------------------------------------------------------------
// This source code is subject to terms and conditions of the Microsoft Public License. A 
// copy of the license can be found in the License.html file at the root of this distribution. 
// If you cannot locate the Microsoft Public License, please send an email to 
// dlr@microsoft.com. By using this source code in any fashion, you are agreeing to be bound 
// by the terms of the Microsoft Public License.
// ----------------------------------------------------------------------------------------------
// You must not remove this notice, or any other, from this software.
// ----------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace DiagProfilerLauncher
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            textBoxStackOutputPath.Text = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
            textBoxStackOutputPath.DoubleClick += new EventHandler(textBoxStackOutputPath_DoubleClick);
        }

        void textBoxStackOutputPath_DoubleClick(object sender, EventArgs e)
        {
            var dialog = new System.Windows.Forms.FolderBrowserDialog();

            string oldPath = textBoxStackOutputPath.Text.Trim();
            if (System.IO.File.Exists(oldPath))
            {
                oldPath = System.IO.Path.GetDirectoryName(oldPath);
            }
            if (System.IO.Directory.Exists(oldPath))
            {
                dialog.SelectedPath = oldPath;
            }
            else
            {
                dialog.SelectedPath = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
            }

            dialog.RootFolder = Environment.SpecialFolder.Personal;
            var dialogResult = dialog.ShowDialog();
            if (dialogResult == System.Windows.Forms.DialogResult.OK)
            {
                if (System.IO.Directory.Exists(dialog.SelectedPath))
                {
                    textBoxStackOutputPath.Text = dialog.SelectedPath;
                }
            }
        }

        ProfileClrApp m_profiler;
        bool Connected 
        { 
            get 
            {
                UpdateStatus();
                return m_profiler != null; 
            } 
        }


        private void UpdateStatus()
        {
            if (m_profiler != null)
            {
                int pId = m_profiler.PId();
                if (pId == 0)
                {
                    m_profiler = null;
                    return;
                }
                try
                {
                    var process = System.Diagnostics.Process.GetProcessById(pId);
                }
                catch (ArgumentException)
                {
                    // PId is not running
                    m_profiler = null;
                }
            }
        }

        private void openExecutableToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (Connected)
            {
                richTextBox1.Text = "Process is already attached";
                return;
            }

            var dialog = new System.Windows.Forms.OpenFileDialog();
            dialog.Filter = "Executable|*.exe";
            dialog.ShowDialog();
            if (dialog.CheckFileExists)
            {
                var profiler = new ProfileClrApp();
                var sb = new StringBuilder();
                sb.AppendLine("Starting process");

                if (!profiler.ClrRuntimeIsV4(dialog.FileName))
                {
                    sb.AppendLine("Executable is not a CLR v4.0 binary");
                    var result = MessageBox.Show("Do you want to launch in v4.0 environment?", "Only CLR 4.0 executables are supported!", MessageBoxButtons.YesNo);
                    if (result != System.Windows.Forms.DialogResult.Yes)
                    {
                        sb.AppendLine("Aborting");
                        richTextBox1.Text = sb.ToString();
                        return;
                    }
                    sb.AppendLine("Process will be started in the V4.0 runtime");
                    sb.AppendLine("This may or may not work");                
                }
                string filenameWithoutExtension = System.IO.Path.Combine(textBoxStackOutputPath.Text, System.IO.Path.GetFileNameWithoutExtension(dialog.FileName));
                string stackOutputPath = filenameWithoutExtension + "_stack.txt";
                string debugOutputPath = filenameWithoutExtension + "_debug.txt";

                if (profiler.StartProcess(dialog.FileName, stackOutputPath, debugOutputPath))
                {
                    sb.AppendLine("Started process");
                    sb.AppendLine("Attaching process");
                    if (profiler.ConnectGUI())
                    {
                        sb.AppendLine("connected GUI");
                        m_profiler = profiler;
                    }
                    else
                    {
                        sb.AppendLine("Failed to connect gui to process");
                    }
                }
                else
                {
                    sb.AppendLine("Failed to start process");
                }

                richTextBox1.Text = sb.ToString();
            }
        }

        private void startToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (Connected)
            {
                if (m_profiler.StartSampling())
                {
                    richTextBox1.Text = "Started Sampling";
                }
                else
                {
                    richTextBox1.Text = "Failed to stop sampling";
                }
            }
        }

        private void stopToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (Connected)
            {
                if (m_profiler.StopSampling())
                {
                    richTextBox1.Text = "Stopped Sampling";
                }
                else
                {
                    richTextBox1.Text = "Failed to stop sampling";
                }
            }
        }

        private void attachToAProcessToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (Connected)
            {
                richTextBox1.Text = "Process is already attached";
                return;
            }

            var processDialog = new FormSelectProcessDialog();
            var dialogResult = processDialog.ShowDialog();
            if (dialogResult == System.Windows.Forms.DialogResult.OK)
            {
                var pId = Convert.ToUInt32(processDialog.SelectedProcessPid);
                var profiler = new ProfileClrApp();
                if (profiler.AttachProfiler(pId))
                {
                    richTextBox1.Text = "Attached to process";
                    m_profiler = profiler;
                    if (profiler.ConnectGUI())
                    {
                        richTextBox1.Text += Environment.NewLine + "Connected to process";
                    }
                    else
                    {
                        richTextBox1.Text += Environment.NewLine + "Failed to connect to process";
                    }
                }
                else
                {
                    richTextBox1.Text = "Failed to attach to process";
                }
            }
        }

        private void sampleCountToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (Connected)
            {
                int count = m_profiler.GetSampleCount();
                richTextBox1.Text = string.Format("Current Sample Count = {0}", count);
            }
        }

        private void sampleRateToolStripMenuItem_Click(object sender, EventArgs e)
        {                    
            if (Connected)
            {
                int rate = m_profiler.GetSampleRate();
                richTextBox1.Text = string.Format("Current Sample Rate = {0} ms", rate);
            }
        }
    }
}
