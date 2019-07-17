#pragma once
#include <queue>
#include <vector>
#include <string>
#include <memory>

namespace nemu::graphics {

	enum class EventType {
		WindowCloseEvent, WindowResizeEvent, DropEvent, KeyTypeEvent,
		KeyReleaseEvent, KeyPressEvent, MouseReleaseEvent, MousePressEvent,
		MouseScrollEvent, MouseMoveEvent
	};

	class Event {
	public:
		virtual EventType GetEventType() = 0;
	};

	class DropEvent : public Event {
		std::vector<std::string> paths;
	public:
		DropEvent(const char** ptr, int count)
		{
			for (int i = 0; i < count; i++)
				paths.push_back(std::string(ptr[i]));
		}

		 EventType GetEventType() { return EventType::DropEvent; }
		 std::vector<std::string>& GetPaths() { return paths; }
		 unsigned int Count() { return paths.size(); }

	};

	class ResizeEvent : public Event {
		int width, height;
	public:
		ResizeEvent(int width, int height)
			: width(width)
			, height(height)
		{}

		EventType GetEventType() { return EventType::WindowResizeEvent; }
		int GetWidth() { return width; }
		int GetHeight() { return height; }
	};

	class KeyPressEvent : public Event {
		int keycode;
	public:
		KeyPressEvent(int code)
			: keycode(code)
		{}

		EventType GetEventType() { return EventType::KeyPressEvent; }
		int GetKey() { return keycode; }
	};

	class KeyReleaseEvent : public Event {
		int keycode;
	public:
		KeyReleaseEvent(int code)
			: keycode(code)
		{}

		EventType GetEventType() { return EventType::KeyReleaseEvent; }
		int GetKey() { return keycode; }
	};

	class EventWrapper {
		std::shared_ptr<Event> event;
	public:

		EventWrapper()
			: event(nullptr)
		{}

		EventWrapper(std::shared_ptr<Event>&& e)
			: event(std::move(e))
		{}

		EventType GetEventType() { return event->GetEventType(); }

		template <class T>
		constexpr operator T&() const
		{
			return *(T*)event.get();
		}

		template <class T>
		T& Get()
		{
			return *(T*)event.get();
		}
	};

}