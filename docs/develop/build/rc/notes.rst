Work notes
==========

These are my notes on the evolution of rc. I used to keep these in separate files on my development
machine, but it makes more sense to include them here. Warning: geeky stuff ahead.

Unlike in most parsers, the unary minus operator is currently not part of the 'data' rules, but
  of 'integer' and 'float'. When expression support is more complete, it may make more sense to put
  this in 'data'. However, 'integer' is used in other places as well and these places allow
  negative numbers too. Maybe we can replace the calls to 'integer' with 'data' and allow the data
  to the integer only (by always trying to cast it, perhaps). Then you can do stuff like
  `resource(10 + 1) 123;` But maybe that goes too far.

When filling in boolean fields of user-defined types or messages, you can do either
  `field = true` or `field = false`. I thought it would be nice if you could also do just
  `field` and `!field`. However, that introduced a reduce/reduce conflict in the parser. You see,
  the name of the field is an IDENT token, but we already have something like that in the 'type'
  rule of 'data'. The parser doesn't know what the IDENT is supposed to be: the name of the boolean
  field or the name of the type. Maybe there is a solution for this by shuffling around the parser
  rules a bit.

Support for the built-in types point, rect, and rgb_color is currently hardcoded into the
  decompiler. The other built-in types -- app_flags, mini_icon, etc -- are not supported at all.
  It would be better to use the type symbol table for this as well. Then the decompiler can also
  support user-defined types (although these type definitions must be provided to the decompiler
  somehow). This is advanced stuff that probably no one will ever use.

The builtin types are added to the symbol table "by hand". You can see this near the bottom of
  'parser.y'. This is a bit cumbersome, so I have devised an alternative. We put the builtin type
  definitions in an rdef script and install this in a "standard include dir", for example:
  ~/config/include/rc. Before it compiles the user's rdef files, the compiler first loads all
  scripts from that standard folder. (This also allows us to use these rdef files for decompiling,
  and users can simply install their own. See above.)

In "auto names" mode, the decompiler currently does not use the enum symbol table. So if two
  resources have the same name and that name is a valid C/C++ identifier, the decompiler will add
  two conflicting symbols to the enum statement. This can also happen when multiple input file
  have conflicting resource IDs.

When you decompile certain apps (BeMail, Slayer) and then compile these rdef files again, the
  archive and message fields in the new .rsrc file are larger than the original's. I think this is
  because rc doesn't add the message fields as "fixedSize" (see the BMessage docs from the BeBook).
  This doesn't really hurt, though, since the BMessage will be properly unflattened regardless.

Right now, archives are treated as messages. Maybe we should give them their own type,
  B_ARCHIVED_OBJECT (type code 'ARCV').

New options, stolen from other resource compilers (rez and beres):

-D --define symbol[=value]
    set the value of symbol to value (or 1 if no value supplied)

--no-names
    do not write any names in resource file

-h
    write resource as C struct in a header file

-d
    dump resource to stdout

Attributes. I would be nice to have a tool that can take text-based descriptions of attributes and
write out an attribute file. Of course, rc is the ideal candidate for this, since it already does
the same for resources. However, resources and attributes are not the same. Attributes are
name/data pairs. The name should be unique. They don't have IDs. They do have a type code, but it
isn't used to identify the attribute. It is probably best to add a new kind of definition:
attribute(). Should this statement allow only simple data, or must attributes be able to handle
flattened messages, arrays, etc too? A source file should either contain resource() statements or
attribute() statements, not both.

User-defined symbolic constants. To keep things simple, adding a #define keyword would suffice,
although this always creates global symbols so there is a chance of name collisions. In addition
(or instead) we can extend user-defined types to have their own (local) defines too. If we use the
#define keyword, we should infer the type of the constant from the data (100 is integer, 10.5 is a
float, etc). This is necessary because we don't have a separate preprocessor step like C/C++ does --
that is why our symbolic constants are typed.
