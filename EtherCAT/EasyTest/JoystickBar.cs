using System;
using System.Drawing;
using System.Windows.Forms;

namespace EasyTest
{
    class JoystickBar : ProgressBar
    {
        public JoystickBar()
        {
            this.SetStyle(ControlStyles.UserPaint, true);
            /*
            //Paintイベントが発生するようにする
            //ダブルバッファリングを有効にする
            base.SetStyle(ControlStyles.UserPaint |
                ControlStyles.AllPaintingInWmPaint |
                ControlStyles.OptimizedDoubleBuffer, true);
                */

        }

        protected override void OnPaint(PaintEventArgs e)
        {
            Rectangle rec = e.ClipRectangle;
            Brush foreBrush = new SolidBrush(this.ForeColor);

            // 枠線描画
            if (ProgressBarRenderer.IsSupported)
                ProgressBarRenderer.DrawHorizontalBar(e.Graphics, e.ClipRectangle);

            int Neutral = Maximum / 2;
            int center = rec.Width / 2;
            // プラスの場合
            if (Value > Neutral)
            {
                int offset = Value - Neutral;
                int width = (rec.Width-4) * offset / Maximum;
                int height = rec.Height - 4;
                e.Graphics.FillRectangle(foreBrush, 2 + center, 2, width, height);
            }
            // マイナスの場合
            else if (Value < Neutral)
            {
                int offset = Neutral - Value;
                int width = (rec.Width - 4) * offset / Maximum;
                int height = rec.Height - 4;
                e.Graphics.FillRectangle(foreBrush, 2 + center - width, 2, width, height);
            }
            // 真ん中の線
            e.Graphics.DrawLine(Pens.DarkGray, center+1, 1, center+1, rec.Height - 1);
        }
    }
}
