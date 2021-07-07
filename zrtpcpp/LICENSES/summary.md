This file contains a summary of files that ZRTP includes but do not use the Apache V2
license. If files are missing in this summary send me a short notice. In this case
consult the file to see its license statement. 


### Files with Copyright (c) 1998-2010, Brian Gladman

Files in the `cryptocommon` directory:
* aes.h
* aes_modes.c
* aescpp.h
* aescrypt.c
* aeskey.c
* aesopt.h
* aestab.c
* aestab.h
* brg_endian.h
* brg_types.h

Files in the `srtp/crypto` directory:
* sha1.c
* sha1.h

Files in the `zrtp/crypto` directory:
* sha2.c
* sha2.h

The license for these files reads:

> ---------------------------------------------------------------------------
> Copyright (c) 1998-2010, Brian Gladman, Worcester, UK. All rights reserved.
> 
> The redistribution and use of this software (with or without changes)
> is allowed without the payment of fees or royalties provided that:
> 
>>   source code distributions include the above copyright notice, this
>>   list of conditions and the following disclaimer;
>> 
>>   binary distributions include the above copyright notice, this list
>>   of conditions and the following disclaimer in their documentation.
> 
> This software is provided 'as is' with no explicit or implied warranties
> in respect of its operation, including, but not limited to, correctness
> and fitness for purpose.
> ---------------------------------------------------------------------------


### The Skein implementation

The files which implement the Skein hash/HMAC algorithm are in the `cryptocommon`:
directory:
* skein.c
* skein.h
* skein_block.c
* skein_port.h

These files have no real copyright statement and use the following license:
> Source code author: Doug Whiting, 2008.
>
> This algorithm and source code is released to the public domain.


### Twofish implementation

Niels Ferguson implemented he `twofish` algorithm the his files have the following
copyright statement: *Copyright (c) 2002 by Niels Ferguson*. The files are also in the
`cryptocommon` directory:
* twofish.c
* twofish.h
* twofish_cfb.c  (no copyright statement in this file)

The license reads:

> The author hereby grants a perpetual license to everybody to 
> use this code for any purpose as long as the copyright message is included 
> in the source code of this or any derived work. 
>  
> Yes, this means that you, your company, your club, and anyone else 
> can use this code anywhere you want. You can change it and distribute it 
> under the GPL, include it in your commercial product without releasing 
> the source code, put it on the web, etc.  
> The only thing you cannot do is remove my copyright message,  
> or distribute any source code based on this implementation that does not  
> include my copyright message.  
>  
> I appreciate a mention in the documentation or credits,  
> but I understand if that is difficult to do. 
> I also appreciate it if you tell me where and why you used my code. 

### Other files with different licenses

These files are either in public domain or use a very liberal license. I checked
the license at the time I copied the files into the ZRTP library. This doc shows
the original licenses (or links to them). 

#### ICU UTF8 handling

Some other files have different licenses.

In directory `common`
* icuUtf.h
* icuUtf8.c

The files use a liberal open source license which is available [here][icuLicense].

#### Base32 encode/decoder

ZRTP uses a Base32 encoding, implemented by Bryce "Zooko" Wilcox-O'Hearn. These
files use the following license:

> Copyright (c) 2002 Bryce "Zooko" Wilcox-O'Hearn Permission is hereby
> granted, free of charge, to any person obtaining a copy of this software to
> deal in this software without restriction, including without limitation the
> rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
> sell copies of this software, and to permit persons to whom this software
> is furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in
> all copies or substantial portions of this software.
>
> THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
> FROM, OUT OF OR IN CONNECTION WITH THIS SOFTWARE OR THE USE OR OTHER
> DEALINGS IN THIS SOFTWARE.

The original Base32 encoder/decoder is in C, I modified/converted it to C++.
 

#### Base64 encoder/decoder

The license is:
> This is part of the libb64 project, and has been placed in the public domain.
> For details, see http://sourceforge.net/projects/libb64


#### Files in the `client` directory

As already stated in the README these files do not belong to the core ZRTP
implementation. They are examples how to use the ZRTP library and usually belong
to other projects which have their own license policy.

Refer to the copyright statements of the source code in these directories, in
particular the sqlite3 sources which have their own license.

[icuLicense]: https://www.unicode.org/copyright.html#License
