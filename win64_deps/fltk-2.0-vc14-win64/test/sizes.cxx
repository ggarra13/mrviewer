#include <fltk/Widget.h>
#include <fltk/Browser.h>
#include <fltk/Button.h>
#include <fltk/CheckButton.h>
#include <fltk/LightButton.h>
#include <fltk/ReturnButton.h>
#include <fltk/Menu.h>
#include <fltk/MenuBar.h>
#include <fltk/PopupMenu.h>
#include <fltk/Choice.h>
#include <fltk/Adjuster.h>
#include <fltk/Clock.h>
#include <fltk/Group.h>
#include <fltk/ColorChooser.h>
#include <fltk/Dial.h>
#include <fltk/Valuator.h>
#include <fltk/ValueInput.h>
#include <fltk/Window.h>
#include <fltk/Input.h>
#include <fltk/Output.h>
#include <fltk/ThumbWheel.h>
#include <fltk/ScrollGroup.h>
#include <fltk/Slider.h>
#include <fltk/Scrollbar.h>
#include <fltk/TabGroup.h>
#include <fltk/Style.h>
#include <fltk/TextBuffer.h>
#include <fltk/TextDisplay.h>
#include <fltk/TextEditor.h>
#include <fltk/Box.h>

#include <stdio.h>

int main() {
  printf("sizeof(fltk::Style) = %lu\n", (unsigned long)sizeof(struct fltk::Style));
  //printf("  sizeof(fltk::Box) = %lu\n", (unsigned long)sizeof(class fltk::Box));
  printf("sizeof(fltk::Widget) = %lu\n", (unsigned long)sizeof(class fltk::Widget));
  printf("  sizeof(fltk::Button) = %lu\n", (unsigned long)sizeof(class fltk::Button));
  printf("    sizeof(fltk::LightButton) = %lu\n", (unsigned long)sizeof(class fltk::LightButton));
  printf("      sizeof(fltk::CheckButton) = %lu\n", (unsigned long)sizeof(class fltk::CheckButton));
  printf("    sizeof(fltk::ReturnButton) = %lu\n", (unsigned long)sizeof(class fltk::ReturnButton));
  printf("  sizeof(fltk::Clock) = %lu\n", (unsigned long)sizeof(class fltk::Clock));
  printf("  sizeof(fltk::Group) = %lu\n", (unsigned long)sizeof(class fltk::Group));
  printf("    sizeof(fltk::Browser) = %lu\n", (unsigned long)sizeof(class fltk::Browser));
  printf("    sizeof(fltk::ColorChooser) = %lu\n", (unsigned long)sizeof(class fltk::ColorChooser));
  printf("    sizeof(fltk::ScrollGroup) = %lu\n", (unsigned long)sizeof(class fltk::ScrollGroup));
  printf("    sizeof(fltk::TabGroup) = %lu\n", (unsigned long)sizeof(class fltk::TabGroup));
  printf("    sizeof(fltk::TextDisplay) = %lu\n", (unsigned long)sizeof(class fltk::TextDisplay));
  printf("      sizeof(fltk::TextEditor) = %lu\n", (unsigned long)sizeof(class fltk::TextEditor));
  printf("    sizeof(fltk::Window) = %lu\n", (unsigned long)sizeof(class fltk::Window));
  printf("  sizeof(fltk::Input) = %lu\n", (unsigned long)sizeof(class fltk::Input));
  printf("  sizeof(fltk::Output) = %lu\n", (unsigned long)sizeof(class fltk::Output));
  printf("  sizeof(fltk::Menu) = %lu\n", (unsigned long)sizeof(class fltk::Menu));
  printf("    sizeof(fltk::Choice) = %lu\n", (unsigned long)sizeof(class fltk::Choice));
  printf("    sizeof(fltk::MenuBar) = %lu\n", (unsigned long)sizeof(class fltk::MenuBar));
  printf("    sizeof(fltk::PopupMenu) = %lu\n", (unsigned long)sizeof(class fltk::PopupMenu));
  printf("  sizeof(fltk::ThumbWheel) = %lu\n", (unsigned long)sizeof(class fltk::ThumbWheel));
  printf("  sizeof(fltk::TextBuffer) = %lu\n", (unsigned long)sizeof(class fltk::TextBuffer));
  printf("  sizeof(fltk::Valuator) = %lu\n", (unsigned long)sizeof(class fltk::Valuator));
  printf("    sizeof(fltk::Adjuster) = %lu\n", (unsigned long)sizeof(class fltk::Adjuster));
  printf("    sizeof(fltk::Dial) = %lu\n", (unsigned long)sizeof(class fltk::Dial));
  printf("    sizeof(fltk::Slider) = %lu\n", (unsigned long)sizeof(class fltk::Slider));
  printf("      sizeof(fltk::Scrollbar) = %lu\n", (unsigned long)sizeof(class fltk::Scrollbar));
  printf("    sizeof(fltk::ValueInput) = %lu\n", (unsigned long)sizeof(class fltk::ValueInput));
  return 0;
}
