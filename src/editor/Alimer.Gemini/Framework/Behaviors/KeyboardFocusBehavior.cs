using System.Windows;
using System.Windows.Input;
using Microsoft.Xaml.Behaviors;

namespace Gemini.Framework.Behaviors
{
    public class KeyboardFocusBehavior : Behavior<FrameworkElement>
    {
        protected override void OnAttached()
        {
            if (!AssociatedObject.IsLoaded)
                AssociatedObject.Loaded += (sender, e) => { Keyboard.Focus(AssociatedObject); };
            else
                Keyboard.Focus(AssociatedObject);

            base.OnAttached();
        }
    }
}
