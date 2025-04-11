#pragma once

template <typename T>
class Bitfield {
public:
	Bitfield() { field = 0; }
	~Bitfield() { }

	void	Clear(void) { field = 0; }
	void	RawSet(uint32_t data) { field = data; }

	void	Set(uint32_t data) { field |= data; }
	void	Clear(uint32_t data) { field &= ~data; }
	void	Unset(uint32_t data) { Clear(data); }
	void	Mask(uint32_t data) { field &= data; }
	void	Toggle(uint32_t data) { field ^= data; }
	void	SetBit(uint32_t data, bool state)
	{
		if (state) Set(data); else Clear(data);
	}

	void	SetField(T data, T mask, T pos) {
		field = (field & ~mask) | (data << pos);
	}

	T		GetField(T mask, T pos) const {
		return (field & mask) >> pos;
	}

	T		Get(void) const { return field; }
	T		GetBit(uint32_t data) const { return field & data; }
	T		Extract(uint32_t bit) const { return (field >> bit) & 1; }
	T		ExtractField(uint32_t shift, uint32_t length)
	{
		return (field >> shift) & (0xFFFFFFFF >> (32 - length));
	}

	bool	IsSet(uint32_t data) const { return GetBit(data) != 0; }
	bool	IsUnSet(uint32_t data) const { return GetBit(data) == 0; }
	bool	IsClear(uint32_t data) const { return IsUnSet(data); }

	T		field;
};

typedef Bitfield <uint8_t>	Bitfield8;
typedef Bitfield <uint16_t>	Bitfield16;
typedef Bitfield <uint32_t>	Bitfield32;	