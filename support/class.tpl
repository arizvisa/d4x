[+ AutoGen5 template h=%s.h cc=%s.cc
#
# $Header: /home/cvs/d4x/support/class.tpl,v 1.1 2004/12/15 02:52:45 zaufi Exp $
#
# This is the autogen template file to produce header and module for new class.
# Check $(top_srcdir)/README.maint for details of usage.
#
+]
/**
 *	\file
 *	$Header: /home/cvs/d4x/support/class.tpl,v 1.1 2004/12/15 02:52:45 zaufi Exp $
 *	\author Copyright (C) 1999-2002 Koshelev Maxim
 *	\brief Class [+classname+] definition
 *	\warning
 *
 *	WebDownloader for X-Window
 *	Copyright (C) 1999-2002 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
[+ CASE (suffix) +][+ == h +][+ (out-move (sprintf "%s.h" (get "classname"))) +]
#ifndef _D4X_[+ (string-upcase (get "classname")) +]_HEADER_
#define _D4X_[+ (string-upcase (get "classname")) +]_HEADER_
[+ == cc +][+ (out-move (sprintf "%s.cc" (get "classname"))) +][+ * +] 
[+ ESAC +]

// Standard includes

// Project specific includes

[+ IF ( == (suffix) "h") +]
#pragma interface

/**
 * \brief [Type brief class description here]
 *
 * [More detailed description here]
 *
 */
class [+classname+]
{
public:
    /// Default constructor
    [+classname+]() {}
    /// Destructor
    virtual ~[+classname+]() {}
};

#endif	// _D4X_[+ (string-upcase (get "classname")) +]_HEADER_
[+ ELSE +]
#pragma implementation
#include "[+ (get "classname") +].h"

[+ ENDIF +]
