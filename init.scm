; DEFMACRO
(set-symbol-macro!
 'defmacro
 (lambda (var pars . body)
   `(set-symbol-macro! ',var (lambda ,pars ,@body))))

; AND2
(defmacro and2 (e1 e2)
  `(if ,e1
       ,e2
     #f))

; DEFINE
(defmacro define (name pars . body)
  (set-symbol-value! name '())
  `(begin
    (set! ,name
          (lambda ,pars ,@body))
    (set-function-name! ,name ',name)))

; FLET
(defmacro flet (definition . body)
  (let ((def (head definition)))
    (let ((name (head def))
          (args (head (tail def)))
          (proc (tail (tail def))))
      `(let ((,name '()))
         (set! ,name
          (lambda ,args ,@proc))
         ,@body))))

; WHILE
(defmacro while (test . body)
  (let ((start (gensym))
        (end (gensym)))
    `(tagbody
      ,start
       (if ,test
           (begin
            ,@body
            (goto ,start))
         (goto ,end))
      ,end
       '())))

; Type Predicates
(define eof? (x)
  (eq? 'teof (type-name (type-of x))))

(define fixnum? (n)
  (eq? 'fixnum (type-name (type-of n))))

(define float? (n)
  (eq? 'float (type-name (type-of n))))

; List Operations
(define first (list)
  (head list))

(define null? (obj)
  (eq? obj '()))

(define rest (list)
  (tail list))

(define map (list fn)
  (if (null? list)
      '()
    (cons (fn (first list))
          (map (rest list) fn))))

(define remove (x list)
  (cond ((null? list) '())
        ((eql? x (first list))
         (remove x (rest list)))
        (else
         (cons (first list)
               (remove x (rest list))))))

(define reduce (list fn)
  (cond ((null? list)
         (signal "Parameter `list' can't be an empty list."))
        ((null? (rest list)) (first list))
        (else
         (fn (first list) (reduce (rest list) fn)))))

; Arithmetic Operations
(defmacro define-bin-arith (name lop hop)
  (let ((n (gensym))
        (m (gensym)))
    `(define ,name (,n ,m)
       (cond ((and2 (fixnum? ,n) (fixnum? ,m)) (,lop ,n ,m))
             ((and2 (fixnum? ,n) (float? ,m)) (,hop (fx->fp ,n) ,m))
             ((and2 (float? ,n) (fixnum? ,m)) (,hop ,n (fx->fp ,m)))
             (else (,hop ,n ,m))))))

;; +
(define-bin-arith bin+ fx+ fp+)

(define + ns
  (cond ((null? ns) 0)
        (else (reduce ns bin+))))

;; -
(define-bin-arith bin- fx- fp-)

(define - (n . ns)
  (cond ((null? ns) (bin- 0 n))
        (else (bin- n (reduce ns bin+)))))

;; *
(define-bin-arith bin* fx* fp*)

(define * ns
  (cond ((null? ns) 1)
        (else (reduce ns bin*))))

;; /
(define-bin-arith bin/ fx/ fp/)

;; =
(define-bin-arith = fx= fp=)

(define / (n . ns)
  (cond ((null? ns) (bin/ 1 n))
        (else (bin/ n (reduce ns bin*)))))

(define < (n m)
  (cond ((> n m) #f)
        ((= n m) #f)
        (else #t)))

(define gcd (n m)
  (if (= m 0)
      n
    (gcd m (mod n m))))

; I/O
;; Output
(define print (x)
  (write-object x *standard-output*)
  (write-char #\newline *standard-output*)
  #t)

; Unix CLI Tools
;; cat
(define cat (file)
  (let ((c (read-char file)))
    (if (eof? c)
        #t
      (begin
        (write-char c *standard-output*)
        (cat file)))))

(define wc (file)
  (flet ((aux (file n)
          (let ((c (read-char file)))
            (if (eof? c)
                n
              (aux file (+ n 1))))))
    (aux file 0)))
