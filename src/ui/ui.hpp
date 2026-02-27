#ifndef SANDBOX_UI_HPP_
#define SANDBOX_UI_HPP_

#include "ui/tft_display.hpp"
#include "ui/led_elements.hpp"
#include "ui/screen_elements.hpp"

namespace sndbx::ui
{
  //====================================================================
  // LED matrix display functions
  //====================================================================

  template<typename T, typename ...Args>
  void draw(LEDMatrixDisplay& display, Args&& ...args)
  {
    static_assert(std::is_base_of_v<LEDUIElement, T>);
    T ledElement(std::forward<Args>(args)..., display.currentFrame());
    ledElement.draw();
    display.setFrameAvailable(true);
  }

  void draw(const LEDUIElement& element, LEDMatrixDisplay& display)
  {
    element.draw();
    display.setFrameAvailable(true);
  }

  template<typename T, typename ...Args>
  void clearAndDraw(LEDMatrixDisplay& display, Args&& ...args)
  {
    display.clear();
    draw<T>(display, std::forward<Args>(args)...);
  }

  void clear(LEDMatrixDisplay& display)
  {
    display.clear();
    display.setFrameAvailable(true);
  }

  //====================================================================
  // TFT functions
  //====================================================================
  void draw(const ScreenElement& element, TFT& display)
  {
    element.draw();
    display.setFrameAvailable(true);
  }

  template<typename T, typename ...Args>
  void draw(TFT& display, Args&& ...args)
  {
    static_assert(std::is_base_of_v<ScreenElement, T>);
    T screenElement(std::forward<Args>(args)..., display.currentFrame());
    screenElement.draw();
    display.setFrameAvailable(true);
  }

  template<typename T, typename ...Args>
  void clearAndDraw(TFT& display, Args&& ...args)
  {
    display.clear();
    draw<T>(display, std::forward<Args>(args)...);
  }

  void clear(TFT& display)
  {
    display.clear();
    display.setFrameAvailable(true);
  }

  //====================================================================
  // Center relative to the screen
  //====================================================================
  void centerX(ScreenElement& element) { element.setX((TFT::width - element.width()) / 2); }

  void centerY(ScreenElement& element) { element.setY((TFT::height - element.height()) / 2); }

  void center(ScreenElement& element) { centerX(element); centerY(element); }

  //====================================================================
  // Center relative to another element
  //====================================================================
  void centerX(ScreenElement& element, const ScreenElement& parent)
  {
    element.setX(parent.x() + (parent.width() - element.width()) / 2);
  }
  
  void centerY(ScreenElement& element, const ScreenElement& parent)
  {
    element.setY(parent.y() + (parent.height() - element.height()) / 2);
  }

  void center(ScreenElement& element, const ScreenElement& parent) 
  { 
    centerX(element, parent); 
    centerY(element, parent); 
  }
}

#endif