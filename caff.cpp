#include "caff.h"

void checksumByte(u32& checksum, const u8 currentByte)
{
	// Signed; values >= 0x80 will be subtracted from the current checksum
	u32 val = (checksum << 4) + static_cast<char>(currentByte);

	u32 val_mask = val & 0xF0000000;
	if (val_mask) {
		/* copy the mask over 0xF00000F0 */
		val_mask |= ((u32)val_mask >> 24);
		val ^= val_mask;
	}

	checksum = val;
}

u32 calculateChecksum(std::vector<char> input)
{
	constexpr size_t sc_headerSize = 64 * 6;

	std::vector<char> data(sc_headerSize);
	data = input;

	u32 checksum = 0;

	size_t index = 0;

	// Checksum the first 20 bytes (magic, version string)
	while (index < 20) {
		checksumByte(checksum, data[index]);
		++index;
	}

	// Skip the stored checksum value
	while (index < 24) {
		checksumByte(checksum, 0);
		++index;
	}

	// Checksum the remaining header data
	while (index < sc_headerSize) {
		checksumByte(checksum, data[index]);
		++index;
	}

	return checksum;
}


unsigned int dword_5C7AC0 = 1;

void decompress(u32 srcBuffer, u32 sizeCompressed, u8* dstBuffer)
{
	dword_5C7AC0 = 1;  // unsure if needed

	// run the decompress routine
	__asm
	{
		push	7801Fh		// SEH callback?
		push	78006h		// sub_78006 
		push	0
		push	dstBuffer
		push	sizeCompressed
		push	srcBuffer
		call	sub_2E5F60
		add		esp, 18h	// game cleanup stack
	}
}

Naked void sub_3AB6B0()
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		push    ebx
		push    esi
		push    edi
		mov     eax, [ebp + 8]
		push    esi
		push    edi
		push    edx
		push    ecx
		push    ebx
		push    ebp
		mov     ebp, eax
		mov     ebx, [ebp + 18h]
		mov     edi, [ebp + 4]
		xor		esi, esi
		mov     si, [ebx]
		shl     esi, 10h
		mov     si, [ebx + 2]
		add     ebx, 4
		mov     ch, 10h
		cmp     ebx, [ebp + 1Ch]
		jnb     loc_3AB8E0
		cmp     edi, [ebp + 8]
		jnb     loc_3AB8E0
		jmp     short loc_3AB717
		nop
		nop
		nop
		nop

		loc_3AB6F0 :
		mov     edx, esi
		mov		[edi], al		// writes the byte to the dst buffer
		shr     edx, 16h
		inc     edi
		movsx   eax, word ptr[ebp + edx * 2 + 3Ch]
		mov     cl, 0Fh
		test    eax, eax
		jl      short loc_3AB73B
		and		cl, al
		shr     eax, 4
		shl     esi, cl
		sub     ch, cl
		jl      short loc_3AB761
		sub     eax, 100h
		jl      short loc_3AB6F0
		jmp     short loc_3AB791

		loc_3AB717 :
		mov     edx, esi
		mov     cl, 0Fh
		shr     edx, 16h
		movsx   eax, word ptr[ebp + edx * 2 + 3Ch]
		test    eax, eax
		jl      short loc_3AB73B
		and		cl, al
		shr     eax, 4
		shl     esi, cl
		sub     ch, cl
		jl      short loc_3AB761
		sub     eax, 100h
		jl      short loc_3AB6F0
		jmp     short loc_3AB791

		loc_3AB73B :
		shl     esi, 0Ah

		loc_3AB73E :
		add     esi, esi
		adc     eax, 0
		movsx   eax, word ptr[ebp + eax * 2 + 1003Ch]
		test    eax, eax
		jl      short loc_3AB73E
		and		cl, al
		shr     eax, 4
		sub     ch, cl
		jl      short loc_3AB761
		sub     eax, 100h
		jl      short loc_3AB6F0
		jmp     short loc_3AB791

		loc_3AB761 :
		cmp     ebx, [ebp + 1Ch]
		jnb     loc_3AB92A
		cmp     edi, [ebp + 8]
		jnb     loc_3AB92A
		mov     cl, ch
		xor		edx, edx
		mov     dx, [ebx]
		neg     cl
		add     ebx, 2
		shl     edx, cl
		add     ch, 10h
		add     esi, edx
		sub     eax, 100h
		jl      loc_3AB6F0

		loc_3AB791 :
		mov     cl, al
		mov     edx, esi
		shr     cl, 4
		or		edx, 1
		shl     esi, cl
		sub     ch, cl
		ror     edx, 1
		xor		cl, 1Fh
		and		eax, 0Fh
		shr     edx, cl
		push    esi
		neg     edx
		cmp     eax, 5
		ja      short loc_3AB821
		lea     esi, [edi + edx]
		cmp     edx, 0FFFFFFFDh
		jnb     short loc_3AB7DB
		cmp     esi, [ebp + 4]
		jb      loc_3AB9E8
		mov     edx, [esi]
		mov		[edi], edx
		mov     edx, [esi + 4]
		mov		[edi + 4], edx
		pop     esi
		lea     edi, [edi + eax + 3]
		test    ch, ch
		jge     loc_3AB717
		jmp     short loc_3AB858

		loc_3AB7DB :
		cmp     esi, [ebp + 4]
		jb      loc_3AB9E8
		mov     dl, [esi]
		mov		[edi], dl
		mov     dl, [esi + 1]
		mov		[edi + 1], dl
		mov     dl, [esi + 2]
		mov		[edi + 2], dl
		mov     dl, [esi + 3]
		mov		[edi + 3], dl
		mov     dl, [esi + 4]
		mov		[edi + 4], dl
		mov     dl, [esi + 5]
		mov		[edi + 5], dl
		mov     dl, [esi + 6]
		mov		[edi + 6], dl
		mov     dl, [esi + 7]
		mov		[edi + 7], dl
		pop     esi
		lea     edi, [edi + eax + 3]
		test    ch, ch
		jge     loc_3AB717
		jmp     short loc_3AB858

		loc_3AB821 :
		cmp     eax, 0Fh
		jz      short loc_3AB882

		loc_3AB826 :
		lea     esi, [edi + edx]
		add     eax, 3
		lea     edx, [edi + eax]
		cmp     esi, [ebp + 4]
		jb      loc_3AB9E8
		xchg    eax, ecx
		cmp     edx, [ebp + 8]
		jnb     loc_3AB989
		rep movsb
		mov     ch, ah
		pop     esi
		cmp     edi, [ebp + 8]
		jnb     loc_3AB99D
		test    ch, ch
		jge     loc_3AB717

		loc_3AB858 :
		cmp     ebx, [ebp + 1Ch]
		jnb     loc_3AB9A5
		cmp     edi, [ebp + 8]
		jnb     loc_3AB9A5
		mov     cl, ch
		xor		edx, edx
		mov     dx, [ebx]
		neg     cl
		add     ebx, 2
		shl     edx, cl
		add     ch, 10h
		add     esi, edx
		jmp     loc_3AB717

		loc_3AB882 :
		xor		eax, eax
		mov     al, [ebx]
		inc     ebx
		cmp     al, 0FFh
		lea     eax, [eax + 0Fh]
		jnz     short loc_3AB826
		xor		eax, eax
		mov     ax, [ebx]
		add     ebx, 2
		cmp     ax, 10Eh
		jnb     short loc_3AB826
		jmp     loc_3AB9E9
		jmp     short loc_3AB8B0	// padding?
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop


		loc_3AB8B0 :
		cmp     edi, [ebp + 0Ch]
		jnb     loc_3AB9FE
		mov     edx, esi
		mov		[edi], al
		shr     edx, 16h
		inc     edi
		movsx   eax, word ptr[ebp + edx * 2 + 3Ch]
		mov     cl, 0Fh
		test    eax, eax
		jl      short loc_3AB904
		and		cl, al
		shr     eax, 4
		shl     esi, cl
		sub     ch, cl
		jl      short loc_3AB92A
		sub     eax, 100h
		jl      short loc_3AB8B0
		jmp     short loc_3AB951

		loc_3AB8E0 :
		mov     edx, esi
		mov     cl, 0Fh
		shr     edx, 16h
		movsx   eax, word ptr[ebp + edx * 2 + 3Ch]
		test    eax, eax
		jl      short loc_3AB904
		and		cl, al
		shr     eax, 4
		shl     esi, cl
		sub     ch, cl
		jl      short loc_3AB92A
		sub     eax, 100h
		jl      short loc_3AB8B0
		jmp     short loc_3AB951

		loc_3AB904 :
		shl     esi, 0Ah

		loc_3AB907 :
		add     esi, esi
		adc     eax, 0
		movsx   eax, word ptr[ebp + eax * 2 + 1003Ch]
		test    eax, eax
		jl      short loc_3AB907
		and		cl, al
		shr     eax, 4
		sub     ch, cl
		jl      short loc_3AB92A
		sub     eax, 100h
		jl      short loc_3AB8B0
		jmp     short loc_3AB951

		loc_3AB92A :
		cmp     ebx, [ebp + 28h]
		jnb     loc_3AB9E9
		mov     cl, ch
		xor		edx, edx
		mov     dx, [ebx]
		neg     cl
		add     ebx, 2
		shl     edx, cl
		add     ch, 10h
		add     esi, edx
		sub     eax, 100h
		jl      loc_3AB8B0

		loc_3AB951 :
		cmp     edi, [ebp + 0Ch]
		jnb     loc_3AB9ED
		mov     cl, al
		mov     edx, esi
		shr     cl, 4
		or		edx, 1
		shl     esi, cl
		sub     ch, cl
		ror     edx, 1
		xor		cl, 1Fh
		and		eax, 0Fh
		shr     edx, cl
		push    esi
		neg     edx
		cmp     eax, 0Fh
		jz      short loc_3AB9C2

		loc_3AB97A :
		lea     esi, [edi + edx]
		add     eax, 3
		lea     edx, [edi + eax]
		cmp     esi, [ebp + 4]
		jb      short loc_3AB9E8
		xchg    eax, ecx

		loc_3AB989 :
		cmp     edx, [ebp + 0Ch]
		jbe     short loc_3AB998
		sub     edx, [ebp + 0Ch]
		sub     ecx, edx
		rep movsb
		pop     esi
		jmp     short loc_3AB9FE

		loc_3AB998 :
		rep movsb
		mov     ch, ah
		pop     esi

		loc_3AB99D :
		test    ch, ch
		jge     loc_3AB8E0

		loc_3AB9A5 :
		cmp     ebx, [ebp + 28h]
		jnb     short loc_3AB9E9
		mov     cl, ch
		xor		edx, edx
		mov     dx, [ebx]
		neg     cl
		add     ebx, 2
		shl     edx, cl
		add     ch, 10h
		add     esi, edx
		jmp     loc_3AB8E0

		loc_3AB9C2 :
		cmp     ebx, [ebp + 14h]
		jnb     short loc_3AB9E8
		xor		eax, eax
		mov     al, [ebx]
		inc     ebx
		cmp     al, 0FFh
		lea     eax, [eax + 0Fh]
		jnz     short loc_3AB97A
		cmp     ebx, [ebp + 20h]
		jnb     short loc_3AB9E8
		xor		eax, eax
		mov     ax, [ebx]
		add     ebx, 2
		cmp     ax, 10Eh
		jnb     short loc_3AB97A
		jmp     short loc_3AB9E9

		loc_3AB9E8 :
		pop     eax

		loc_3AB9E9 :
		xor		eax, eax
		jmp     short loc_3ABA03

		loc_3AB9ED :
		cmp     edi, [ebp + 0]
		jnz     short loc_3AB9FE
		test    eax, eax
		jnz     short loc_3AB9FE
		mov     eax, 1
		mov		[ebp + 34h], eax

		loc_3AB9FE :
		mov     eax, 1

		loc_3ABA03 :
		mov		[ebp + 30h], eax
		mov		[ebp + 2Ch], ebx
		mov		[ebp + 10h], edi
		pop     ebp
		pop     ebx
		pop     ecx
		pop     edx
		pop     edi
		pop     esi
		pop     edi
		pop     esi
		pop     ebx
		pop     ebp
		retn
		}
};

Naked void sub_3AB500()
{
	__asm
	{
		sub     esp, 80h
		push    ebp
		mov     ebp, [esp + 88h]
		push    esi
		push    edi
		mov     ecx, 10h
		xor		eax, eax
		lea     edi, [esp + 0Ch]
		rep		stosd
		mov     ecx, 100h

		loc_3AB522 :
		movzx   eax, byte ptr[ecx + ebp - 1]
		dec     ecx
		mov     edx, eax
		and		edx, 0Fh
		mov     edi, [esp + edx * 4 + 0Ch]
		lea     edx, [esp + edx * 4 + 0Ch]
		sar     eax, 4
		inc     edi
		mov		[edx], edi
		mov     edx, [esp + eax * 4 + 0Ch]
		lea     eax, [esp + eax * 4 + 0Ch]
		inc     edx
		test    ecx, ecx
		mov		[eax], edx
		jnz     short loc_3AB522
		mov     eax, [esp + 0Ch]
		cmp     eax, 200h
		jz      loc_3AB69D
		cmp     eax, 1FFh
		jz      short loc_3AB588
		mov     ecx, 10h
		lea     esi, [esp + 0Ch]
		lea     edi, [esp + 4Ch]
		rep movsd
		xor		eax, eax
		mov     ecx, 0Fh

		loc_3AB576 :
		add     eax, [esp + ecx * 4 + 0Ch]
		test    al, 1
		jnz     short loc_3AB588
		sar     eax, 1
		dec     ecx
		jnz     short loc_3AB576
		cmp     eax, 1
		jz      short loc_3AB594

		loc_3AB588 :
		pop     edi
		pop     esi
		xor		eax, eax
		pop     ebp
		add     esp, 80h
		retn

		loc_3AB594 :
		xor		ecx, ecx
		mov     eax, 1
		jmp     short loc_3AB5A0
		lea     ecx, [ecx + 0]
		nop

		loc_3AB5A0 :
		mov     edi, [esp + eax * 4 + 0Ch]
		add     edi, ecx
		mov		[esp + eax * 4 + 0Ch], edi
		inc     eax
		cmp     eax, 10h
		mov     ecx, edi
		jl      short loc_3AB5A0
		mov     edi, [esp + 48h]
		mov     eax, 2000h
		jmp     short loc_3AB5C0
		lea     ecx, [ecx + 0]
		nop

		loc_3AB5C0 :
		sub     eax, 10h
		mov     ecx, eax
		sar     ecx, 5
		movzx   ecx, byte ptr[ecx + ebp]
		shr     ecx, 4
		test    ecx, ecx
		jz      short loc_3AB5E4
		mov     edx, [esp + ecx * 4 + 0Ch]
		dec     edx
		mov     esi, eax
		or		esi, ecx
		mov		[esp + ecx * 4 + 0Ch], edx
		mov		[ebx + edx * 2], si

		loc_3AB5E4 :
		sub     eax, 10h
		mov     edx, eax
		sar     edx, 5
		xor		ecx, ecx
		mov     cl, [edx + ebp]
		and		ecx, 0Fh
		jz      short loc_3AB607
		mov     edx, [esp + ecx * 4 + 0Ch]
		dec     edx
		mov     esi, eax
		or		esi, ecx
		mov		[esp + ecx * 4 + 0Ch], edx
		mov		[ebx + edx * 2], si

		loc_3AB607 :
		test    eax, eax
		jnz     short loc_3AB5C0
		mov     ecx, 800h
		mov     eax, ecx
		mov     esi, 0Fh

		loc_3AB617 :
		cmp     eax, ecx
		mov     edx, ecx
		jle     short loc_3AB634
		lea		ecx, [ecx + 0]
		nop

		loc_3AB620 :
		sub     eax, 2
		mov     ebp, eax
		dec     ecx
		or		ebp, 8000h
		cmp     eax, edx
		mov		[ebx + ecx * 2], bp
		jg      short loc_3AB620

		loc_3AB634 :
		mov     eax, [esp + esi * 4 + 4Ch]
		dec     eax
		js      short loc_3AB64E
		inc     eax
		lea     esp, [esp + 0]
		nop

		loc_3AB640 :
		mov     bp, [ebx + edi * 2 - 2]
		dec     edi
		dec     ecx
		dec     eax
		mov		[ebx + ecx * 2], bp
		jnz     short loc_3AB640

		loc_3AB64E :
		dec     esi
		cmp     esi, 0Ah
		mov     eax, edx
		jg      short loc_3AB617
		cmp     eax, ecx
		mov     edx, 400h
		jle     short loc_3AB674
		nop

		loc_3AB660 :
		sub     eax, 2
		mov     esi, eax
		dec		edx
		or		esi, 8000h
		cmp     eax, ecx
		mov		[ebx + edx * 2], si
		jg      short loc_3AB660

		loc_3AB674 :
		test    edi, edi
		jle     short loc_3AB69D

		loc_3AB678 :
		movzx   esi, word ptr[ebx + edi * 2 - 2]
		dec     edi
		mov     ecx, esi
		and		ecx, 0Fh
		mov     ebp, 400h
		sar     ebp, cl
		mov     eax, edx
		sub     eax, ebp
		mov     edi, edi

		loc_3AB690 :
		dec     edx
		cmp     edx, eax
		mov		[ebx + edx * 2], si
		jnz     short loc_3AB690
		test    edi, edi
		jg      short loc_3AB678

		loc_3AB69D :
		pop     edi
		pop     esi
		mov     eax, 1
		pop     ebp
		add     esp, 80h
		retn
	}
};

Naked void sub_2E8D30()
{
	__asm
	{
		test	eax, eax
		jz      short locret_2E8D48
		cmp     eax, 0x1000
		jnb     short loc_2E8D49
		neg     eax
		add     eax, esp
		add     eax, 0x4
		test	[eax], eax
		xchg    eax, esp
		mov     eax, [eax]
		push    eax

		locret_2E8D48 :
		retn

		loc_2E8D49 :
		push    ecx
		lea     ecx, [esp + 0x8]

		loc_2E8D4E :
		sub     ecx, 0x1000
		sub     eax, 0x1000
		test	[ecx], eax
		cmp     eax, 0x1000
		jnb     short loc_2E8D4E
		sub     ecx, eax
		mov     eax, esp
		test	[ecx], eax
		mov     esp, ecx
		mov     ecx, [eax]
		mov     eax, [eax + 0x4]
		push    eax
		retn
	}
};

Naked void sub_3ABA20()
{
	__asm
	{
		mov     eax, 1140h
		call    sub_2E8D30
		mov     ecx, [esp + 1144h]
		mov     edx, [esp + 1150h]
		push    esi
		lea     eax, [esp + 4]
		and		eax, 0xFF
		lea     esi, [esp + 104h]
		sub     esi, eax
		mov		[esi + 18h], ecx
		mov     ecx, [esp + 1150h]
		cmp     edx, ecx
		push    edi
		jle     short loc_3ABA63
		mov		[esp + 1158h], ecx
		mov     edx, ecx

		loc_3ABA63 :
		mov     edi, [esp + 1160h]
		cmp     edi, ecx
		jz      loc_3ABB9A
		test    edx, edx
		jz      loc_3ABB9A
		cmp     ecx, edi
		jl      loc_3ABB8C
		test    edi, edi
		jl      loc_3ABB8C
		cmp     ecx, 105h
		jle     loc_3ABB8C
		cmp     edi, 105h
		jl      loc_3ABB8C
		cmp     ecx, 10000h
		jg      loc_3ABB8C
		test    edx, edx
		jl      loc_3ABB8C
		mov     eax, [esp + 1150h]
		push    ebx
		add     ecx, eax
		push    ebp
		mov     ebp, [esp + 1164h]
		mov		[esi], ecx
		lea     ecx, [edi + ebp]
		lea     edi, [ecx - 1]
		add     edx, eax
		mov		[esi + 20h], edi
		mov		[esi + 28h], edi
		mov     edi, edx
		sub     edi, eax
		cmp     edi, 108h
		lea     ebx, [ecx - 3]
		mov		[esi + 4], eax
		mov		[esi + 0Ch], edx
		mov		[esi + 14h], ecx
		mov		[esi + 24h], ebx
		mov		[esi + 8], eax
		jbe     short loc_3ABAFF
		add     edx, 0FFFFFEF8h
		mov		[esi + 8], edx

		loc_3ABAFF :
		mov     eax, [esp + 1154h]
		mov     edx, ecx
		sub     edx, eax
		cmp     edx, 0E8h
		mov		[esi + 1Ch], eax
		jbe     short loc_3ABB1E
		add     ecx, 0FFFFFF18h
		mov		[esi + 1Ch], ecx

		loc_3ABB1E :
		push    ebp
		lea     ebx, [esi + 3Ch]
		call    sub_3AB500
		add     esp, 4
		test    eax, eax
		jnz     short loc_3ABB3E

		loc_3ABB2E :
		pop     ebp
		pop     ebx
		pop     edi
		or		eax, 0FFFFFFFFh
		pop     esi
		add     esp, 1140h
		retn    18h

		loc_3ABB3E :
		xor		edi, edi
		add     ebp, 100h
		push    esi
		mov		[esi + 18h], ebp
		mov		[esi + 30h], edi
		mov		[esi + 34h], edi
		call    sub_3AB6B0
		mov     eax, [esi + 30h]
		add     esp, 4
		cmp     eax, edi
		jz      short loc_3ABB2E
		mov     eax, [esi + 0Ch]
		cmp		[esi + 10h], eax
		ja      short loc_3ABB2E
		mov     ecx, [esi + 2Ch]
		cmp     ecx, [esi + 14h]
		ja      short loc_3ABB2E
		cmp     eax, [esi]
		jnz     short loc_3ABB78
		cmp		[esi + 34h], edi
		jz      short loc_3ABB2E

		loc_3ABB78 :
		mov     eax, [esp + 1160h]
		pop     ebp
		pop     ebx
		pop     edi
		pop     esi
		add     esp, 1140h
		retn    18h

		loc_3ABB8C :
		pop     edi
		or		eax, 0FFFFFFFFh
		pop     esi
		add     esp, 1140h
		retn    18h

		loc_3ABB9A :
		pop     edi
		mov     eax, edx
		pop     esi
		add     esp, 1140h
		retn    18h
	}
};

Naked void sub_2E5F60()
{
	_asm
	{
		mov     eax, [esp + 0x8]
		push    ebp
		mov     ebp, [esp + 0x10]
		push    esi
		mov     esi, [esp + 0x0C]
		add     eax, esi
		cmp     esi, eax
		mov		[esp + 0x0C], eax
		jnb     short loc_2E5FAE
		push    ebx
		push    edi
		lea		ebx, [ebx + 0]

		loc_2E5F80 :
		mov     edi, [esi]
		mov     ebx, [esi + 0x4]
		mov     ecx, dword_5C7AC0
		add     esi, 0x4
		push    ebx
		add     esi, 0x4
		push    esi
		push    0x10000
		push    edi
		push    ebp
		push    ecx
		call    sub_3ABA20
		mov     eax, [esp + 0x14]
		add     esi, ebx
		add     ebp, edi
		cmp     esi, eax
		jb      short loc_2E5F80
		pop     edi
		pop     ebx

		loc_2E5FAE :
		pop     esi
		pop     ebp
		ret
	}
};