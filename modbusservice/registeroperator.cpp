#include <stdio.h>
#include "registeroperator.h"


void RegisterOperator::Show(map<unsigned,unsigned>& m)
{
	for(Iterator i = m.begin(); i != m.end(); i++)
	{
		RegisterOperator x(i->first, i->second);
		printf("<%08X, %08X>(%d, %d)\n", i->first, i->second, x.GetOffset(), x.GetCount());
	}
}
RegisterOperator::RegisterOperator(void):key(0),value(0)
{
}
RegisterOperator::RegisterOperator(int i, int c):key(i),value(c)
{
}
RegisterOperator::RegisterOperator(int s, int f, int o, int c)
{
	SetSlave(s);
	SetFcode(f);
	SetOffset(o);
	SetCount(c);
	SetInterval(0xfff);
	SetComId(0x00);
}
RegisterOperator::RegisterOperator(int s, int f, int o, int c, int i)
{
	SetSlave(s);
	SetFcode(f);
	SetOffset(o);
	SetCount(c);
	SetInterval(i);
	SetComId(0x00);
}
RegisterOperator::RegisterOperator(int s, int f, int o, int c, int i, int comid)
{
	SetSlave(s);
	SetFcode(f);
	SetOffset(o);
	SetCount(c);
	SetInterval(i);
	SetComId(comid);
}
void RegisterOperator::SetKey(int i)
{
	key = i;
}
void RegisterOperator::SetValue(int v)
{
	value = v;
}
void RegisterOperator::SetSlave(unsigned char slave)
{
	key = (key & ~(0xff << 24));
	key = (key | (slave << 24));
}
void RegisterOperator::SetFcode(unsigned char fcode)
{
	key = (key & ~(0xff << 16));
	key = (key | (fcode << 16));
}
void RegisterOperator::SetOffset(unsigned short offset)
{
	key = (key & ~(0xffff <<  0));
	key = (key | (offset <<  0));
}
void RegisterOperator::SetCount(unsigned short count)
{
	value &= ~(0xffff << 16);
	value |= (0xffff << 16) & (count << 16);
}
void RegisterOperator::SetInterval(unsigned short interval)
{
	value &= ~(0xfff << 4);
	value |= (0xfff << 4) & (interval << 4);
}
void RegisterOperator::SetComId(unsigned char comid)
{
	value &= ~(0xf << 0);
	value |= (0xf << 0) & (comid << 0);
}
unsigned char RegisterOperator::GetSlave(void)
{
	return 0xff & (key >> 24);
}
unsigned char RegisterOperator::GetFcode(void)
{
	return 0xff & (key >> 16);
}
unsigned short RegisterOperator::GetOffset(void)
{
	return 0xffff & (key >> 0);
}
unsigned short RegisterOperator::GetCount(void)
{
	return 0xffff & (value >> 16);
}
unsigned short RegisterOperator::GetInterval(void)
{
	return 0xfff & (value >> 4);
}
unsigned char RegisterOperator::GetComId(void)
{
	return 0xf & (value >> 0);
}
int RegisterOperator::GetKey(void)
{
	return key;
}
int RegisterOperator::GetValue(void)
{
	return value;
}

void RegisterOperator::Add(map<unsigned,unsigned>& m, int key, int value)
{
	Merge(m, key, value);
	Split(m);
}
void RegisterOperator::Merge(map<unsigned,unsigned>& m, int key, int value)
{
	for(Iterator i = m.begin(); i != m.end(); i++)
	{
		RegisterOperator n(key, value);
		RegisterOperator o(i->first, i->second);

		if( n.GetCount() < 1 )
		{
			continue;
		}
		if( n.GetSlave() != o.GetSlave() )
		{
			continue;
		}
		if( n.GetFcode() != o.GetFcode() )
		{
			continue;
		}
		int A = o.GetOffset();
		int B = A + o.GetCount() - 1;
		int C = n.GetOffset();
		int D = C + n.GetCount() - 1;
		switch( GetABCD(A, B, C, D) )
		{
			case ABCD:
				if( (B + 1) == C || (B == C) )
				{
					o.SetCount(D - A + 1);
					m.erase(o.GetKey());
					Merge(m, o.GetKey(), o.GetValue());
					return;
				}
				break;

			case ACBD:
				o.SetCount(D - A + 1);
				m.erase(o.GetKey());
				Merge(m, o.GetKey(), o.GetValue());
				return;

			case ACDB:
				return;

			case CDAB:
				if( (D + 1) == A || (D == A) )
				{
					m.erase(o.GetKey());
					n.SetCount(B - C + 1);
					Merge(m, n.GetKey(), n.GetValue());
					return;
				}
				break;

			case CADB:
				m.erase(o.GetKey());
				n.SetCount(B - C + 1);
				Merge(m, n.GetKey(), n.GetValue());
				return;

			case CABD:
				m.erase(o.GetKey());
				Merge(m, n.GetKey(), n.GetValue());
				return;
		}
	}
	m[key] = value;
}
void RegisterOperator::Merge(map<unsigned,unsigned>& m)
{
	map<unsigned,unsigned> tmp = m;
	for(Iterator i = tmp.begin(); i != tmp.end(); i++)
	{
		Merge(m, i->first, i->second);
	}
}
void RegisterOperator::Split(map<unsigned,unsigned>& m)
{
	for(Iterator i = m.begin(); i != m.end(); i++)
	{
		RegisterOperator o(i->first, i->second);

		switch(o.GetFcode())
		{
			case 0x01:
			case 0x02:
				if( o.GetCount() > CountX01 )
				{
					RegisterOperator x(o.GetKey(), o.GetValue());
					x.SetCount(CountX01);
					m[x.GetKey()] = x.GetValue();
					x.SetOffset(x.GetOffset() + CountX01);
					x.SetCount(o.GetCount() - CountX01);
					m[x.GetKey()] = x.GetValue();
					Split(m);
					continue;
				}
				break;
			case 0x03:
			case 0x04:
				if( o.GetCount() > CountX03 )
				{
					RegisterOperator x(o.GetKey(), o.GetValue());
					x.SetCount(CountX03);
					m[x.GetKey()] = x.GetValue();
					x.SetOffset(x.GetOffset() + CountX03);
					x.SetCount(o.GetCount() - CountX03);
					m[x.GetKey()] = x.GetValue();
					Split(m);
					continue;
				}
				break;
		}
	}
}
void RegisterOperator::Del(map<unsigned,unsigned>& m, int key, int value)
{
	Delx(m, key, value);
	if( m.empty() == false )
	{
		Merge(m);
		Split(m);
	}
}
void RegisterOperator::Delx(map<unsigned,unsigned>& m, int key, int value)
{
	for(Iterator i = m.begin(); i != m.end(); i++)
	{
		RegisterOperator n(key, value);
		RegisterOperator o(i->first, i->second);

		if( n.GetCount() < 1 )
		{
			return;
		}
		if( n.GetSlave() != o.GetSlave() )
		{
			continue;
		}
		if( n.GetFcode() != o.GetFcode() )
		{
			continue;
		}
		int A = o.GetOffset();
		int B = A + o.GetCount() - 1;
		int C = n.GetOffset();
		int D = C + n.GetCount() - 1;
		switch( GetABCD(A, B, C, D) )
		{
			case ABCD:
				if( B == C )
				{
					m.erase(o.GetKey());
					if( (B - A) > 0 )
					{
						o.SetCount(B - A);
						Add(m, o.GetKey(), o.GetValue());
					}
					Delx(m, key, value);
					return;
				}
				break;

			case ACBD:
				m.erase(o.GetKey());
				if( (C - A) > 0 )
				{
					o.SetCount(C - A + 1);
					Add(m, o.GetKey(), o.GetValue());
				}
				Delx(m, key, value);
				return;

			case ACDB:
				m.erase(o.GetKey());
				if( (C - A) > 0 )
				{
					o.SetCount(C - A);
					m[o.GetKey()] = o.GetValue();
				}
				if( (B - D) > 0 )
				{
					n.SetOffset(D + 1);
					n.SetCount(B - D);
					Add(m, n.GetKey(), n.GetValue());
					Delx(m, key, value);
					return;
				}
				Delx(m, key, value);
				return;

			case CDAB:
				if( D == A )
				{
					m.erase(o.GetKey());
					if( (B - A) > 0 )
					{
						o.SetOffset(A + 1);
						o.SetCount(B - A);
						Add(m, o.GetKey(), o.GetValue());
					}
					Delx(m, key, value);
					return;
				}
				break;

			case CADB:
				m.erase(o.GetKey());
				if( (B - D) > 0 )
				{
					o.SetOffset(D + 1);
					o.SetCount(B - D);
					Add(m, o.GetKey(), o.GetValue());
				}
				Delx(m, key, value);
				return;

			case CABD:
				m.erase(o.GetKey());
				Delx(m, key, value);
				return;
		}
	}
}
int RegisterOperator::GetABCD(int A, int B, int C, int D)
{
	if( (A <= B) && (B <= C) && (C <= D) )
	{
		return ABCD;
	}
	if( (A <= C) && (C <= B) && (B <= D) )
	{
		return ACBD;
	}
	if( (A <= C) && (C <= D) && (D <= B) )
	{
		return ACDB;
	}
	if( (C <= D) && (D <= A) && (A <= B) )
	{
		return CDAB;
	}
	if( (C <= A) && (A <= D) && (D <= B) )
	{
		return CADB;
	}
	if( (C <= A) && (A <= B) && (B <= D) )
	{
		return CABD;
	}
	return 0;
}
string RegisterOperator::RangeToString(int range)
{
	switch(range)
	{
		case ABCD:
			return "ABCD";
		case ACBD:
			return "ACBD";
		case ACDB:
			return "ACDB";
		case CDAB:
			return "CDAB";
		case CADB:
			return "CADB";
		case CABD:
			return "CABD";
	}
	return "abcd";
}

#if 0
int main(void)
{
	map<unsigned,unsigned> m;
	RegisterOperator a(1, 3, 0, 10, 100);
	RegisterOperator b(1, 3, 20, 3, 100);
	RegisterOperator c(1, 3, 0, 200, 100);
	RegisterOperator d(1, 3, 3, 1, 100);

	RegisterOperator().Add(m, a.GetKey(), a.GetValue());
	RegisterOperator().Add(m, b.GetKey(), b.GetValue());
	RegisterOperator().Add(m, c.GetKey(), c.GetValue());
	RegisterOperator().Del(m, b.GetKey(), b.GetValue());
	RegisterOperator().Del(m, d.GetKey(), d.GetValue());
	//RegisterOperator().Add(m, b.GetKey(), b.GetValue());


	for(map<unsigned,unsigned>::iterator i = m.begin(); i != m.end(); i++)
	{
		RegisterOperator x(i->first, i->second);
		printf("<%08X, %08X>(%d, %d)\n", i->first, i->second, x.GetOffset(), x.GetCount());
	}

	return 0;
}
#endif
