(set! defmacro
      (macro (var pars . body)
        `(set! ,var (macro ,pars ,@body))))

(defmacro define (var pars . body)
  `(begin
    (set! ,var
          (lambda ,pars ,@body))
    (set-function-name! ,var ',var)))

(define abs (x)
  (if (> 0 x)
      (bin- 0 x)
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
       (set! n (bin- n 1))
       (set! lst (tail lst))
       (goto nth-m)))))

(defmacro defun (name pars . body)
  `(set! ,name
    (name-lambda ,name ,pars ,@body)))

(define nth (n lst)
  (cond ((= n 0) (head lst))
        (else (nth (bin- n 1) (tail lst)))))

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

(define length (list)
  (let ((aux (lambda (n list)
               (tagbody
                aux
                 (if (null? list)
                     n
                   (begin
                    (set! n (+ n 1))
                    (set! list (tail list))
                    (goto aux)))))))
    (aux 0 list)))

(define fixnum? (n)
  (eq? 'fixnum (type-name (type-of n))))

(define float? (n)
  (eq? 'float (type-name (type-of n))))

(defmacro define-bin-arith (name opl oph)
  (let ((n (gensym))
        (m (gensym)))
    `(define ,name (,n ,m)
       (cond ((and2 (fixnum? ,n) (fixnum? ,m)) (,opl ,n ,m))
             ((and2 (fixnum? ,n) (float? ,m)) (,oph (fx->fp ,n) ,m))
             ((and2 (float? ,n) (fixnum? ,m)) (,oph ,n (fx->fp ,m)))
             (else (,oph ,n ,m))))))

(define-bin-arith bin+ fx+ fp+)
(define-bin-arith bin- fx- fp-)
(define-bin-arith bin* fx* fp*)
(define-bin-arith bin/ fx/ fp/)

(defmacro or2 (e1 e2)
  (let ((v (gensym)))
    `(let ((,v ,e1))
       (if ,v
           ,v
         ,e2))))

(define reduce (fn list)
  (cond ((null? list) (signal "Parameter list can't be an empty list."))
        ((null? (tail list))
         (head list))
        (else
         (fn (head list) (reduce fn (tail list))))))

(define + ns
  (cond ((null? ns) 0)
        (else (reduce bin+ ns))))

(define * ns
  (cond ((null? ns) 1)
        (else (reduce bin* ns))))

(define - (n . ns)
  (cond ((null? ns) (bin- 0 n))
        (else (bin- n (reduce bin+ ns)))))

(define / (n . ns)
  (cond ((null? ns) (bin/ 1 n))
        (else (bin/ n (reduce bin* ns)))))

(define get-ks (kvs)
  (if (null? kvs)
      '()
    (cons (head kvs)
          (get-ks (tail (tail kvs))))))

(define get-vs (kvs)
  (if (null? kvs)
      '()
    (cons (head (tail kvs))
          (get-vs (tail (tail kvs))))))

(define ngensym (n)
  (if (= n 0)
      '()
    (cons (gensym)
          (ngensym (- n 1)))))

(define map2 (fn list1 list2)
  (if (or2 (null? list1) (null? list2))
      '()
    (cons (fn (head list1) (head list2))
          (map2 fn (tail list1) (tail list2)))))

(define pset!-fv (kvs)
  (let ((tmps (ngensym (/ (length kvs) 2)))
        (ks (get-ks kvs))
        (vs (get-vs kvs)))
    `(let ,(map2 list tmps vs)
       ,@(map2 (lambda (k tmp)
                 `(set! ,k ,tmp))
               ks tmps))))

(defmacro pset! kvs
  (pset!-fv kvs))

(defun gcd (n m)
  (if (= m 0)
      n
    (gcd m (mod n m))))

