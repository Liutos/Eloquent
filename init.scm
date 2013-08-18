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

(define nth-m (n lst)
  (tagbody
   nth-m
    (if (= n 0)
        (head lst)
      (begin
       (set! n (- n 1))
       (set! lst (tail lst))
       (goto nth-m)))))

(defmacro defun (name pars . body)
  `(set! ,name
    (name-lambda ,name ,pars ,@body)))

(defun nth (n lst)
  (if (= n 0)
      (head lst)
    (nth (- n 1) (tail lst))))

(define pair? (x)
  (if (eq? 'pair (type-name (type-of x)))
      #t
    #f))

(define atom? (x)
  (if (pair? x)
      #f
    #t))

(define butlast (list)
  (if (eq? list '())
      '()
    (if (eq? (tail list) '())
        '()
      (cons (head list)
            (butlast (tail list))))))

(defmacro and2 (e1 e2)
  `(if ,e1
       ,e2
     #f))
