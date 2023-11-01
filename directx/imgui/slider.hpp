#pragma once

#include "child.hpp"
#include "util.hpp"

namespace ig {
	class slider_float : public child {
	private:
		float _value, _min, _max;
		std::string _text;
		callback_t _callback;
	public:
		void render() override;

		slider_float();

		float get() const;
		float get_min() const;
		float get_max() const;
		std::string get_text() const;
		callback_t get_callback() const;

		void set(float value);
		void set_min(float min);
		void set_max(float max);
		void set_text(const std::string& text);
		void set_callback(const callback_t& callback);
	};

	class slider_int : public child {
	private:
		int _value, _min, _max;
		std::string _text;
		callback_t _callback;
	public:
		void render() override;

		slider_int();

		int get() const;
		int get_min() const;
		int get_max() const;
		std::string get_text() const;
		callback_t get_callback() const;

		void set(int value);
		void set_min(int min);
		void set_max(int max);
		void set_text(const std::string& text);
		void set_callback(const callback_t& callback);
	};
}
