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


(library (ikarus numerics complex-numbers)
  (export
    make-rectangular		make-polar
    real-part			imag-part
    magnitude			angle
    complex-conjugate

;;; --------------------------------------------------------------------

    $magnitude-fixnum		$magnitude-bignum	$magnitude-ratnum
    $magnitude-flonum		$magnitude-compnum	$magnitude-cflonum

    $angle-fixnum		$angle-bignum		$angle-ratnum
    $angle-flonum		$angle-compnum		$angle-cflonum

    $complex-conjugate-compnum	$complex-conjugate-cflonum

    $make-rectangular)
  (import (except (ikarus)
		  make-rectangular	make-polar
		  real-part		imag-part
		  angle			magnitude)
    (except (ikarus system $compnums)
	    $make-rectangular)
    (ikarus system $fx)
    (ikarus system $bignums)
    (ikarus system $ratnums)
    (ikarus system $flonums)
    (rename (only (ikarus generic-arithmetic) #;(ikarus system $numerics)
		  $abs-fixnum
		  $abs-bignum
		  $abs-ratnum
		  $abs-flonum
		  $atan2-real-real)
	    ($abs-fixnum		$magnitude-fixnum)
	    ($abs-bignum		$magnitude-bignum)
	    ($abs-ratnum		$magnitude-ratnum)
	    ($abs-flonum		$magnitude-flonum))
    (vicare arguments validation)
    (only (vicare syntactic-extensions)
	  cond-numeric-operand))


;;;; helpers

(define dummy #f)

(define (%error-not-number who Z)
  (assertion-violation who "expected number object as argument" Z))


(define ($make-rectangular rep imp)
  ;;REP and IMP can be any combination of real numbers.  If IMP is exact
  ;;zero: the returned value is REP, a real.
  ;;
  (cond ((eq? imp 0)
	 rep)
	((and (flonum? rep)
	      (flonum? imp))
	 ($make-cflonum rep imp))
	(else
	 ($make-compnum rep imp))))


(module (make-rectangular)

  (define who 'make-rectangular)

  (define (make-rectangular rep imp)
    (cond-numeric-operand imp
      ((flonum?)
       (cond-numeric-operand rep
	 ((flonum?)		($make-cflonum rep imp))
	 ((real? exact?)	($make-compnum rep imp))
	 (else			(%error-real rep))))
      ((zero?)
       rep)
      ((real? exact?)
       (cond-numeric-operand rep
	 ((flonum?)		($make-compnum rep imp))
	 ((real? exact?)	($make-compnum rep imp))
	 (else			(%error-real rep))))
      (else
       (%error-imag imp))))

  (define (%error-real x)
    (assertion-violation who "invalid real part argument" x))

  (define (%error-imag x)
    (assertion-violation who "invalid imag part argument" x))

  #| end of module |# )


(define (make-polar mag angle)
  (define who 'make-polar)
  (with-arguments-validation (who)
      ((real	mag)
       (real	angle))
    ($make-rectangular (* mag (cos angle))
		       (* mag (sin angle)))))


(module (magnitude
	 $magnitude-compnum
	 $magnitude-cflonum)
  (define who 'magnitude)

  (define (magnitude x)
    (cond-numeric-operand x
      ((compnum?)	($magnitude-compnum x))
      ((cflonum?)	($magnitude-cflonum x))
      ((fixnum?)	($magnitude-fixnum  x))
      ((bignum?)	($magnitude-bignum  x))
      ((ratnum?)	($magnitude-ratnum  x))
      ((flonum?)	($magnitude-flonum  x))
      (else
       (%error-not-number who x))))

  (define ($magnitude-compnum x)
    (let ((x.rep ($compnum-real x))
	  (x.imp ($compnum-imag x)))
      (sqrt (+ (square x.rep) (square x.imp)))))

  (define ($magnitude-cflonum x)
    ($flsqrt ($fl+ ($flsquare ($cflonum-real x))
		   ($flsquare ($cflonum-imag x)))))

  #| end of module: magnitude |# )


(module (angle
	 $angle-fixnum		$angle-bignum		$angle-ratnum
	 $angle-flonum		$angle-compnum		$angle-cflonum)
  (define who 'angle)
  (define PI  (acos -1))

  (define (angle x)
    (cond-numeric-operand x
      ((compnum?)	($angle-compnum x))
      ((cflonum?)	($angle-cflonum x))
      ((fixnum?)	($angle-fixnum  x))
      ((bignum?)	($angle-bignum  x))
      ((ratnum?)	($angle-ratnum  x))
      ((flonum?)	($angle-flonum  x))
      (else
       (%error-not-number who x))))

  (define ($angle-fixnum x)
    (cond (($fxpositive? x)	0)
	  (($fxnegative? x)	PI)
	  (else
	   (assertion-violation who "undefined for 0"))))

  (define ($angle-bignum x)
    (if ($bignum-positive? x) 0 PI))

  (define ($angle-ratnum x)
    (let ((n ($ratnum-n x)))
      (if (> n 0) 0 PI)))

  (define ($angle-flonum x)
    (if (or ($flpositive?      x)
	    ($flzero?/positive x))
	0.0
      PI))

  (define ($angle-compnum x)
    (let ((x.rep ($compnum-real x))
	  (x.imp ($compnum-imag x)))
      (atan x.imp x.rep)))

  (define ($angle-cflonum x)
    (let ((x.rep ($cflonum-real x))
	  (x.imp ($cflonum-imag x)))
      ($atan2-real-real x.rep x.imp)))

  #| end of module: angle |# )


(define (real-part x)
  (define who 'real-part)
  (cond-numeric-operand x
    ((compnum?)	($compnum-real x))
    ((cflonum?)	($cflonum-real x))
    ((fixnum?)	x)
    ((bignum?)	x)
    ((ratnum?)	x)
    ((flonum?)	x)
    (else
     (%error-not-number who x))))

(define (imag-part x)
  (define who 'imag-part)
  (cond-numeric-operand x
    ((fixnum?)	0)
    ((bignum?)	0)
    ((ratnum?)	0)
    ((flonum?)	0)
    ((compnum?)	($compnum-imag x))
    ((cflonum?)	($cflonum-imag x))
    (else
     (%error-not-number who x))))


(module (complex-conjugate
	 $complex-conjugate-compnum	$complex-conjugate-cflonum)
  (define who 'complex-conjugate)

  (define (complex-conjugate Z)
    (cond-numeric-operand Z
      ((compnum?)	($complex-conjugate-compnum Z))
      ((cflonum?)	($complex-conjugate-cflonum Z))
      ((fixnum?)	Z)
      ((bignum?)	Z)
      ((ratnum?)	Z)
      ((flonum?)	Z)
      (else
       (%error-not-number who Z))))

  (define ($complex-conjugate-compnum Z)
    (let ((Z.rep ($compnum-real Z))
	  (Z.imp ($compnum-imag Z)))
      ($make-rectangular Z.rep (- Z.imp))))

  (define ($complex-conjugate-cflonum Z)
    (let ((Z.rep ($cflonum-real Z))
	  (Z.imp ($cflonum-imag Z)))
      ($make-cflonum Z.rep ($fl- Z.imp))))

  #| end of module: complex-conjugate |# )


;;;; done

)

;;; end of file
;; Local Variables:
;; eval: (put 'cond-numeric-operand 'scheme-indent-function 1)
;; End:
