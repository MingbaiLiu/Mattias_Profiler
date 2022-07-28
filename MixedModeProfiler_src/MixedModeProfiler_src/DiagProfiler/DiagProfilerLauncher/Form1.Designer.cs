namespace DiagProfilerLauncher
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.richTextBox1 = new System.Windows.Forms.RichTextBox();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openExecutableToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.attachToAProcessToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.samplingToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.startToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.stopToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.sampleCountToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.sampleRateToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.textBoxStackOutputPath = new System.Windows.Forms.TextBox();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // richTextBox1
            // 
            this.richTextBox1.Location = new System.Drawing.Point(151, 12);
            this.richTextBox1.Name = "richTextBox1";
            this.richTextBox1.Size = new System.Drawing.Size(298, 126);
            this.richTextBox1.TabIndex = 3;
            this.richTextBox1.Text = "";
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.samplingToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(471, 24);
            this.menuStrip1.TabIndex = 5;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openExecutableToolStripMenuItem,
            this.attachToAProcessToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // openExecutableToolStripMenuItem
            // 
            this.openExecutableToolStripMenuItem.Name = "openExecutableToolStripMenuItem";
            this.openExecutableToolStripMenuItem.Size = new System.Drawing.Size(175, 22);
            this.openExecutableToolStripMenuItem.Text = "Open Executable";
            this.openExecutableToolStripMenuItem.Click += new System.EventHandler(this.openExecutableToolStripMenuItem_Click);
            // 
            // attachToAProcessToolStripMenuItem
            // 
            this.attachToAProcessToolStripMenuItem.Name = "attachToAProcessToolStripMenuItem";
            this.attachToAProcessToolStripMenuItem.Size = new System.Drawing.Size(175, 22);
            this.attachToAProcessToolStripMenuItem.Text = "Attach to a Process";
            this.attachToAProcessToolStripMenuItem.Click += new System.EventHandler(this.attachToAProcessToolStripMenuItem_Click);
            // 
            // samplingToolStripMenuItem
            // 
            this.samplingToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.startToolStripMenuItem,
            this.stopToolStripMenuItem,
            this.sampleCountToolStripMenuItem,
            this.sampleRateToolStripMenuItem});
            this.samplingToolStripMenuItem.Name = "samplingToolStripMenuItem";
            this.samplingToolStripMenuItem.Size = new System.Drawing.Size(69, 20);
            this.samplingToolStripMenuItem.Text = "Sampling";
            // 
            // startToolStripMenuItem
            // 
            this.startToolStripMenuItem.Name = "startToolStripMenuItem";
            this.startToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
            this.startToolStripMenuItem.Text = "Start";
            this.startToolStripMenuItem.Click += new System.EventHandler(this.startToolStripMenuItem_Click);
            // 
            // stopToolStripMenuItem
            // 
            this.stopToolStripMenuItem.Name = "stopToolStripMenuItem";
            this.stopToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
            this.stopToolStripMenuItem.Text = "Stop";
            this.stopToolStripMenuItem.Click += new System.EventHandler(this.stopToolStripMenuItem_Click);
            // 
            // sampleCountToolStripMenuItem
            // 
            this.sampleCountToolStripMenuItem.Name = "sampleCountToolStripMenuItem";
            this.sampleCountToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
            this.sampleCountToolStripMenuItem.Text = "Sample Count";
            this.sampleCountToolStripMenuItem.Click += new System.EventHandler(this.sampleCountToolStripMenuItem_Click);
            // 
            // sampleRateToolStripMenuItem
            // 
            this.sampleRateToolStripMenuItem.Name = "sampleRateToolStripMenuItem";
            this.sampleRateToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
            this.sampleRateToolStripMenuItem.Text = "Sample Rate";
            this.sampleRateToolStripMenuItem.Click += new System.EventHandler(this.sampleRateToolStripMenuItem_Click);
            // 
            // textBoxStackOutputPath
            // 
            this.textBoxStackOutputPath.Location = new System.Drawing.Point(49, 144);
            this.textBoxStackOutputPath.Name = "textBoxStackOutputPath";
            this.textBoxStackOutputPath.ReadOnly = true;
            this.textBoxStackOutputPath.Size = new System.Drawing.Size(400, 20);
            this.textBoxStackOutputPath.TabIndex = 6;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(471, 181);
            this.Controls.Add(this.textBoxStackOutputPath);
            this.Controls.Add(this.richTextBox1);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Form1";
            this.Text = "DiagProfiler Launcher";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.RichTextBox richTextBox1;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openExecutableToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem attachToAProcessToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem samplingToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem startToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem stopToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem sampleCountToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem sampleRateToolStripMenuItem;
        private System.Windows.Forms.TextBox textBoxStackOutputPath;
    }
}

