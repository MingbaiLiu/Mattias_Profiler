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
using System.Diagnostics;

namespace DiagProfilerLauncher
{
    public partial class FormSelectProcessDialog : Form
    {
        public class ProcessItem
        {
            public int Id { get; set; }
            public string ProcessName { get; set; }
            public string DisplayName
            {
                get
                {
                    return string.Format("{0}  {1}", this.Id, this.ProcessName);
                }
            }
        }

        public int SelectedProcessPid { get; private set; }

        public FormSelectProcessDialog()
        {
            InitializeComponent();
            var processes = System.Diagnostics.Process.GetProcesses();
            var viewableList = processes.Select(n => new ProcessItem() { Id = n.Id, ProcessName = n.ProcessName }).ToList();
            listBoxRunningProcesses.DataSource = viewableList;
            listBoxRunningProcesses.DisplayMember = "ProcessName";
            listBoxRunningProcesses.ValueMember = "DisplayName";
            listBoxRunningProcesses.DoubleClick += new EventHandler(listBoxRunningProcesses_DoubleClick);
        }

        void listBoxRunningProcesses_DoubleClick(object sender, EventArgs e)
        {
            var process = listBoxRunningProcesses.SelectedItem as ProcessItem;
            SelectedProcessPid = process.Id;
            this.DialogResult = System.Windows.Forms.DialogResult.OK;
        }
    }
}
