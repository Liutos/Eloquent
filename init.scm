(set! defmacro
      (macro (var pars . body)
        `(set! ,var (macro ,pars ,@body))))

(defmacro define (var pars . body)
  `(set! ,var
         (lambda ,pars ,@body)))

(define abs (x)
  (if (> 0 x)
      (- 0 x)
      x))

(define < (x y)
  (cond ((> x y) #f)
        ((= x y) #f)
        (else #t)))
