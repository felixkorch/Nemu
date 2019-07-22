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

		 virtual EventType GetEventType() override { return EventType::DropEvent; }
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

		virtual EventType GetEventType() override { return EventType::WindowResizeEvent; }
		int GetWidth() { return width; }
		int GetHeight() { return height; }
	};

	class KeyPressEvent : public Event {
		int keycode;
	public:
		KeyPressEvent(int code)
			: keycode(code)
		{}

		virtual EventType GetEventType() override { return EventType::KeyPressEvent; }
		int GetKey() { return keycode; }
	};

	class KeyReleaseEvent : public Event {
		int keycode;
	public:
		KeyReleaseEvent(int code)
			: keycode(code)
		{}

		virtual EventType GetEventType() override { return EventType::KeyReleaseEvent; }
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

		template <typename T>
		operator T&()
		{
			return *(T*)event.get();
		}

		Event* operator->() const
		{
			return event.get();
		}

		bool operator==(std::nullptr_t)
		{
			return event == nullptr;
		}
	};

	template <typename EventClass, typename ...Args>
	inline static EventWrapper WrapEvent(Args... args)
	{
		return EventWrapper(std::make_shared<EventClass>(args...));
	}

}