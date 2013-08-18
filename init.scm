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

(define null? (x) (eq? '() x))

(define map (seq fn)
  (if (null? seq)
      '()
    (cons (fn (head seq))
          (map (tail seq) fn))))

(define ify-clause (key clause)
  `((eql? ,key ,(head clause)) ,@(tail clause)))

(defmacro case (keyform . clauses)
  (let ((key (gensym)))
    `(let ((,key ,keyform))
       (cond
        ,@(map clauses
               (lambda (clause)
                 (if (eq? 'else (head clause))
                     `(#t ,@(tail clause))
                   `((eql? ,key ',(head clause)) ,@(tail clause)))))))))

(defmacro typecase (keyform . clauses)
  `(case (type-name (type-of ,keyform)) ,@clauses))

(define nth (n lst)
  (tagbody
   nth
    (if (= n 0)
        (head lst)
      (begin
       (set! n (- n 1))
       (set! lst (tail lst))
       (goto nth)))))

(define pair? (x)
  (if (eq? 'pair (type-name (type-of x)))
      #t
    #f))

(define atom? (x)
  (if (pair? x)
      #f
    #t))