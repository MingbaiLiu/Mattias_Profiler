namespace DiagProfilerLauncher
{
    partial class FormSelectProcessDialog
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
            this.listBoxRunningProcesses = new System.Windows.Forms.ListBox();
            this.SuspendLayout();
            // 
            // listBoxRunningProcesses
            // 
            this.listBoxRunningProcesses.FormattingEnabled = true;
            this.listBoxRunningProcesses.Location = new System.Drawing.Point(12, 17);
            this.listBoxRunningProcesses.Name = "listBoxRunningProcesses";
            this.listBoxRunningProcesses.Size = new System.Drawing.Size(260, 238);
            this.listBoxRunningProcesses.TabIndex = 0;
            // 
            // FormSelectProcessDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.listBoxRunningProcesses);
            this.Name = "FormSelectProcessDialog";
            this.Text = "FormSelectProcessDialog";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox listBoxRunningProcesses;
    }
}