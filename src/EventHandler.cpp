#include <windows.h>
#include "EventHandler.h"
#include "Input.h"
#include "Application.h"
#include "Camera.h"

void EventHandler::HandleEvent(EventType type, unsigned long p1, unsigned long p2) {
  switch (type) {
    case KEYDOWN_EVENT: {
      Input::HandleKeyPressEvent((int) p1);
      if (p1 == VK_ESCAPE) {
        // Lose focus on escape
        SetFocus(nullptr);
      }
      break;
    }
    case KEYUP_EVENT:
      Input::HandleKeyReleaseEvent((int) p1);
      break;
    case MOUSEMOVE_EVENT:
//      LPPOINT mousePos;
//      GetCursorPos(mousePos);
//      if (mousePos)
//        Input::HandleMouseMoveEvent(mousePos->x, mousePos->y);
      Camera::Displace((int) p1, (int) p2);
      break;
    case RESIZE_EVENT:
      if (Application::window && p1 > 0 && p2 > 0) {
        Application::renderer->SetProjection(Application::window->width, Application::window->height);
      }
      break;
    case QUIT_EVENT:
      Application::running = false;
      break;
    default:
      return;
  }
}
