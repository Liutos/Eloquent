(set! define
      (macro (var pars . body)
        `(set! ,var (lambda ,pars ,@body))))

(define abs (x)
  (if (> 0 x)
      (- 0 x)
      x))

(define < (x y)
  (if (> x y)
      #f
      (if (= x y)
          #f
          #t)))
