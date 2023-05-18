========================================================================
         TeX Live Guide Translation (Serbian, Cyrillic Script)
  --------------------------------------------------------------------
      Original: The TeX Live Guide: TeX Live 2020, Karl Berry (ed.)
        Translator: Nikola Le??i?? <nikola.lecic@anthesphoria.net>
========================================================================

To compile texlive-sr.tex, you need:

1. LuaLaTeX (lualatex);

2. the following packages, apart from those required to build texlive-en:
   microtype, polyglossia, metalogo, caption, relsize, titling, multitoc,
   etoolbox and grffile;

3. the following fonts:

   Old Standard >= 2.0.2 (by Alexey Kryukov) [1]
   Computer Modern Unicode >= 0.6.3a (by Andrey Panov) [2]

   [1] http://www.thessalonica.org.ru/en/oldstandard.html
   [2] http://canopus.iacp.dvo.ru/~panov/cm-unicode/
       http://sourceforge.net/projects/cm-unicode/

   (these fonts are also included in TeX Live >= 2010 -- please read
   the TeX Live Guide, section `System font configuration for XeTeX and
   LuaTeX' on how to configure your system);

4. BSD or GNU make.

To produce texlive-sr.pdf, type `make realclean pdf'.

Since tex4ht currently does not fully support LuaTeX, the HTML version
of this documentation is produced similarly to that of the English
original, but some handiwork was necessary to simplify .tex and .sty, to
change encoding to cp1251 and back to utf-8, and to alter the generated
CSS. Therefore you cannot produce HTML from the source using Makefile.

If you have any problems, suggestions or questions, please contact the
translator.

Permission is granted to copy, distribute and/or modify this translation
under the terms of the adapted version of FreeBSD documentation licence.
The text of the licence is included in texlive-sr.tex and at the end of
resulting PDF/HTML files.

http://www.gnu.org/licenses/license-list.html#FreeDocumentationLicenses
http://www.freebsd.org/copyright/freebsd-doc-license.html

