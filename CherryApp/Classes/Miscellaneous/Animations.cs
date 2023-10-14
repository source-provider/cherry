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
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Media.Animation;
using System.Runtime.InteropServices;
using System.Diagnostics;
using Extensions;

namespace CherryApp.Classes.Miscellaneous
{
    public static class Animations
    {

        public static async void Fade(DependencyObject Object, double time, double fade_val = 1.0)
        {
            DoubleAnimation doubleAnimation = new DoubleAnimation();
            Storyboard storyboard = new Storyboard();

            double start = (double)Object.GetValue(UIElement.OpacityProperty);



            doubleAnimation.From = new double?(start);
            doubleAnimation.To = new double?(fade_val);
            doubleAnimation.Duration = new Duration(TimeSpan.FromSeconds(time));
            DoubleAnimation FadeIn = doubleAnimation;
            Storyboard.SetTarget((DependencyObject)FadeIn, Object);
            await Task.Delay(100);
            Storyboard.SetTargetProperty((DependencyObject)FadeIn, new PropertyPath("Opacity", new object[1]
            {
                (object) 1
            }));
            storyboard.Children.Add((Timeline)FadeIn);
            storyboard.Begin();
            await Task.Delay(TimeSpan.FromSeconds(time) + TimeSpan.FromSeconds(0.1));
            storyboard.Children.Clear();

            storyboard = (Storyboard)null;
            FadeIn = (DoubleAnimation)null;
        }

        public static async void FadeOut(DependencyObject Object, double time)
        {
            DoubleAnimation doubleAnimation = new DoubleAnimation();
            Storyboard storyboard = new Storyboard();

            double start = (double)Object.GetValue(UIElement.OpacityProperty);

            doubleAnimation.From = new double?(start);
            doubleAnimation.To = new double?(0.0);
            doubleAnimation.Duration = new Duration(TimeSpan.FromSeconds(time));
            DoubleAnimation FadeOut = doubleAnimation;

            Storyboard.SetTarget((DependencyObject)FadeOut, Object);
            await Task.Delay(100);
            Storyboard.SetTargetProperty((DependencyObject)FadeOut, new PropertyPath("Opacity", new object[1]
            {
                (object) 1
            }));
            storyboard.Children.Add((Timeline)FadeOut);
            storyboard.Begin();
            await Task.Delay(TimeSpan.FromSeconds(time) + TimeSpan.FromSeconds(0.1));
            storyboard.Children.Clear();

            storyboard = (Storyboard)null;
            FadeOut = (DoubleAnimation)null;
        }

        public static async void TweenContentToColor(Label TextLabel, Color Color)
        {
            ColorAnimation Animation = new ColorAnimation();
            Animation.From = TextLabel.Foreground.ToColor();
            Animation.To = Color;
            Animation.Duration = new Duration(TimeSpan.FromSeconds(0.25));

            PropertyPath ColorTargetPath = new PropertyPath("(Label.Foreground).(SolidColorBrush.Color)");
            Storyboard storyboard = new Storyboard();
            Storyboard.SetTarget(Animation, TextLabel);
            Storyboard.SetTargetProperty(Animation, ColorTargetPath);
            storyboard.Children.Add(Animation);
            storyboard.Begin();

            await Task.Delay(TimeSpan.FromSeconds(0.25) + TimeSpan.FromSeconds(0.1));
            storyboard.Children.Clear();

            Animation = (ColorAnimation)null;
            ColorTargetPath = (PropertyPath)null;
        }
    }

}
