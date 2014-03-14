;;;Ikarus Scheme -- A compiler for R6RS Scheme.
;;;Copyright (C) 2006,2007,2008  Abdulaziz Ghuloum
;;;Modified by Marco Maggi <marco.maggi-ipsu@poste.it>
;;;
;;;This program is free software:  you can redistribute it and/or modify
;;;it under  the terms of  the GNU General  Public License version  3 as
;;;published by the Free Software Foundation.
;;;
;;;This program is  distributed in the hope that it  will be useful, but
;;;WITHOUT  ANY   WARRANTY;  without   even  the  implied   warranty  of
;;;MERCHANTABILITY  or FITNESS FOR  A PARTICULAR  PURPOSE.  See  the GNU
;;;General Public License for more details.
;;;
;;;You should  have received  a copy of  the GNU General  Public License
;;;along with this program.  If not, see <http://www.gnu.org/licenses/>.


#!vicare
(library (psyntax compat)
  (export
    define*				define-constant
    case-define				case-define*
    case-lambda*			lambda*
    define-record			define-auxiliary-syntaxes
    define-inline			define-syntax-rule
    define-fluid-override		unwind-protect
    receive				receive-and-return
    module				import
    begin0				define-values

    __who__				brace

    make-struct-type			struct?
    struct-type-descriptor?		struct-type-field-names

    make-parameter			parametrise
    symbol-value			set-symbol-value!
    keyword?				would-block-object?
    unbound-object?			bwp-object?
    bignum?				gensym
    vector-append			vector-exists
    add1				sub1
    pretty-print			pretty-print*
    fprintf				debug-print
    void				port-id
    console-error-port			all-identifiers?
    string-empty?

    ;; compiler related operations
    eval-core

    ;; runtime options
    option.verbose-about-libraries?
    option.strict-r6rs
    option.enable-arguments-validation?
    option.descriptive-labels
    option.print-loaded-libraries
    option.cache-compiled-libraries

    ;; interpreting the result of reading annotated sources
    annotation?				annotation-expression
    annotation-stripped			annotation-source
    annotation-textual-position

    ;; source position condition objects
    make-source-position-condition	source-position-condition?
    source-position-byte		source-position-character
    source-position-line		source-position-column
    source-position-port-id

    label-binding			set-label-binding!
    remove-location

    ;; symbol property lists
    putprop				getprop
    remprop				property-list

    ;; error handlers
    library-version-mismatch-warning
    library-stale-warning
    procedure-argument-violation
    warning

    ;; system stuff
    file-modification-time

    ;; library names and version numbers
    library-name?
    library-version-numbers?		library-version-number?
    library-name-decompose
    library-name->identifiers		library-name->version
    library-name-identifiers=?		library-name=?
    library-name<?			library-name<=?
    library-version=?
    library-version<?			library-version<=?

    ;; library references and conformity
    library-reference?			library-version-reference?
    library-sub-version-reference?	library-sub-version?
    library-reference-decompose
    library-reference->identifiers
    library-reference->version-reference
    library-reference-identifiers=?
    conforming-sub-version-and-sub-version-reference?
    conforming-version-and-version-reference?
    conforming-library-name-and-library-reference?

    ;; unsafe bindings
    $car $cdr
    $fx= $fx< $fx> $fx<= $fx>= $fxadd1
    $fxzero? $fxpositive? $fxnonnegative?
    $vector-ref $vector-set! $vector-length)
  (import (vicare)
    (only (ikarus.compiler)
	  eval-core)
    (prefix (rename (only (vicare options)
			  verbose?
			  verbose-about-libraries?
			  strict-r6rs
			  descriptive-labels
			  print-loaded-libraries
			  cache-compiled-libraries
			  vicare-built-with-arguments-validation-enabled)
		    (vicare-built-with-arguments-validation-enabled
		     enable-arguments-validation?))
	    option.)
    (ikarus library-utils)
    (only (ikarus.posix)
	  ;;This is used by INCLUDE to register the modification time of
	  ;;the files included  at expand-time.  Such time is  used in a
	  ;;STALE-WHEN test.
	  file-modification-time)
    ;;NOTE Let's try  to import the unsafe operations  from the built-in
    ;;libraries, when possible, rather  that using external libraries of
    ;;macros.
    (only (vicare system $symbols)
	  $unintern-gensym)
    (only (vicare system $fx)
	  $fx= $fx< $fx> $fx<= $fx>= $fxadd1
	  $fxzero? $fxpositive? $fxnonnegative?)
    (only (vicare system $pairs)
	  $car $cdr)
    (only (vicare system $vectors)
	  $vector-ref $vector-set! $vector-length))


(define (library-version-mismatch-warning name depname filename)
  (when (option.verbose?)
    (fprintf (current-error-port)
	     "*** Vicare warning: library ~s has an inconsistent dependency \
              on library ~s; file ~s will be recompiled from source.\n"
	     name depname filename)))

(define (library-stale-warning name filename)
  (when (option.verbose?)
    (fprintf (current-error-port)
	     "*** Vicare warning: library ~s is stale; file ~s will be \
              recompiled from source.\n"
	     name filename)))

(define-syntax define-record
  (syntax-rules ()
    [(_ name (field* ...) printer)
     (begin
       (define-struct name (field* ...))
       (module ()
	 (set-rtd-printer! (type-descriptor name)
			   printer)))]
    [(_ name (field* ...))
     (define-struct name (field* ...))]))

(define (set-label-binding! label binding)
  (set-symbol-value! label binding))

(define (label-binding label)
  (and (symbol-bound? label) (symbol-value label)))

(define (remove-location x)
  ($unintern-gensym x))


;;;; done

;; #!vicare
;; (define dummy
;;   (foreign-call "ikrt_print_emergency" #ve(ascii "psyntax.compat")))

)

;;; end of file
