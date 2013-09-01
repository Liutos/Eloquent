(set-symbol-macro!
 'defmacro
 (lambda (var pars . body)
   `(set-symbol-macro! ',var (lambda ,pars ,@body))))

(defmacro define (name pars . body)
  (set-symbol-value! name '())
  `(begin
    (set! ,name
          (lambda ,pars ,@body))
    (set-function-name! ,name ',name)))

(define < (n m)
  (cond ((> n m) #f)
        ((= n m) #f)
        (else #t)))
