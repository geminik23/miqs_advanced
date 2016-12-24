#pragma once

#include "miqs.adv.base.h"

namespace miqs
{


/**	pitch shifter

pitch_shift<value_type, 3> ps;
ps.delay_capacity(96000);
ps.samplerate(48000);

* for better quality, recommended to set delay_capacity longer than samplerate
*/
template <typename _Ty, size_t _TapCount>
struct pitch_shift: _base_pointer_type<_Ty>, _base_trans_arg1<_Ty>
{
	enum { tap_count = _TapCount };

	pitch_shift():pitch_shift(48000) {}
	pitch_shift(uint32_t sr) { delay_capacity(sr); window_ratio(m_win_ratio); samplerate(sr); }

	void reset()
	{
		for (size_t i{}; i < tap_count; ++i) m_delaypos[i] = (double)i / tap_count;
		m_delayline.reset();
		m_delayline.delay(m_delaypos[0]);
	}

	void samplerate(uint32_t sr) noexcept
	{
		m_samplerate = sr;
		if (m_samplerate > delay_capacity()) delay_capacity(m_samplerate);
		reset();
	}
	uint32_t samplerate() const noexcept { return m_samplerate; }

	void delay_capacity(size_t size) noexcept { m_delayline.capacity(size); }
	size_t delay_capacity() const noexcept { return m_delayline.capacity(); }

	void pitch_ratio(double r) noexcept { m_pitch_ratio = r; }
	double pitch_ratio() const noexcept { return m_pitch_ratio; }

	void window_ratio(double r) noexcept
	{
		m_win_ratio = r;
		size_t old = m_win_size;
		m_win_size = static_cast<int>(m_win_ratio* m_delayline.capacity());

		if (old == m_win_size) return;

		this->reset_buffer(m_win_size);

		miqs::phase_info<int> phase(0, 1);
		miqs::ptr_fill_window_hanning(
			ptr_begin(m_winbuff), ptr_at(m_winbuff, m_win_size),
			phase, m_win_size);
	}

	double window_ratio() const noexcept { return m_win_ratio; }

	miqs::interpolation_type interpolation_type() const noexcept { return m_delayline.interpolation_type(); }
	void interpolation_type(miqs::interpolation_type type) noexcept { m_delayline.interpolation_type(type); }


	void operator()(_ptr_arg<value_type> arg) { this->operator()(arg.first, arg.last); }
	void operator()(_ptr_arg<value_type> iarg, _ptr_arg<value_type> oarg) { this->operator()(iarg.first, iarg.last, oarg.first); }
	void operator()(pointer first, pointer last) { this->operator()(first, last, first); }
	void operator()(pointer first, pointer last, pointer dst) { for (; first != last;)*dst++ = this->operator()(*first++); }

	result_type operator()(argument_type in)
	{
		result_type result{};
		m_rate = ((1.0 - m_pitch_ratio) / m_win_ratio) / m_samplerate;

		for (size_t i{}; i < tap_count; ++i)
		{
			m_delayline.delay(((m_delaypos[i] * m_win_ratio)* m_samplerate) + 1);
			result += m_winbuff[(int)(m_delaypos[i] * m_win_size)] * m_delayline.peek();

			m_delaypos[i] += m_rate;
			MIQS_PTR_MODULAR_DOWN(m_delaypos[i], 1.0);
			MIQS_PTR_MODULAR_UP(m_delaypos[i], 1.0);
		}
		m_delayline.push(in);

		return result * 2.0 / tap_count;
	}

private:
	void reset_buffer(size_t size) { m_winbuff.reset(size); }

	/* properties */
	uint32_t m_samplerate{ 48000 };
	double m_pitch_ratio{ 1 };
	double m_win_ratio{ 0.1 };
	array<value_type> m_winbuff;
	fractional_delay<value_type> m_delayline;


	/* internal */
	double m_delaypos[tap_count];
	double m_rate; //internal
	size_t m_win_size{}; //internal
};





}