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

; OR2
(defmacro or2 (e1 e2)
  (let ((tmp (gensym)))
    `(let ((,tmp ,e1))
       (if ,tmp
           ,tmp
           ,e2))))

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
(define is-type? (x type)
  (eq? type (type-name (type-of x))))

(define bignum? (x)
  (is-type? x 'bignum))

(define eof? (x)
  (is-type? x 'teof))

(define fixnum? (x)
  (is-type? x 'fixnum))

(define float? (x)
  (is-type? x 'float))

(define function? (x)
  (is-type? x 'function))

(define mpflonum? (x)
  (is-type? x 'mpflonum))

(define primitive? (x)
  (is-type? x 'primitive))

(define symbol? (x)
  (is-type? x 'symbol))

(define vector? (x)
  (is-type? x 'vector))

; List Operations
(define first (list)
  (head list))

(define null? (obj)
  (eq? obj '()))

(define rest (list)
  (tail list))

(define second (list)
  (first (tail list)))

(define append (l1 l2)
  (cond ((null? l1) l2)
        ((null? l2) l1)
        (#t (cons (first l1)
                  (append (rest l1) l2)))))

(define list xs
  xs)

(define each (list fn)
  (if (null? list)
      '()
    (begin
      (fn (first list))
      (each (rest list) fn))))

(define map (list fn)
  (if (null? list)
      '()
    (cons (fn (first list))
          (map (rest list) fn))))

(define reduce (list fn)
  (cond ((null? list)
         (signal "Parameter `list' can't be an empty list."))
        ((null? (rest list)) (first list))
        (else
         (fn (first list) (reduce (rest list) fn)))))

(define remove (x list)
  (cond ((null? list) '())
        ((eql? x (first list))
         (remove x (rest list)))
        (else
         (cons (first list)
               (remove x (rest list))))))

(define reverse (lst)
  (cond ((null? lst) '())
        ((null? (tail lst)) lst)
        (else 
         (append (reverse (tail lst))
                 `(,(head lst))))))

;; DOLIST
(defmacro dolist (decl . body)
  (let ((var (first decl))
        (list (second decl)))
    `(each ,list (lambda (,var) ,@body))))

; Structure
(defmacro defstruct (name . fields)
  `(make-structure ',name ',fields))

; String
(define write-line (str)
  (write-string str *standard-output*)
  (write-char #\newline *standard-output*))

; Symbol
(define fbound? (x)
  (and2 (bound? x)
        (or2 (function? (symbol-value x))
             (primitive? (symbol-value x)))))

; Vector
(define vector-last (x)
  (vector-ref x (- (vector-length x) 1)))

; Arithmetic Operations
(define nt-convert (n src dest)
  (cond ((and2 (eq? src 'fixnum) (eq? dest 'float))
         (fx->fp n))
        ((and2 (eq? src 'fixnum) (eq? dest 'bignum))
         (fx->bg n))
        (else (signal "Unknown convert rule."))))

(define < (n m)
  (cond ((> n m) #f)
        ((= n m) #f)
        (else #t)))

(define gcd (n m)
  (if (= m 0)
      n
    (gcd m (mod n m))))

;; DOTIMES
(defmacro dotimes (decl . body)
  (let ((var (first decl))
        (num (second decl))
        (aux (gensym)))
    `(flet ((,aux (,var)
              (if (< ,var ,num)
                  (begin
                    ,@body
                    (,aux (+ ,var 1)))
                '())))
       (,aux 0))))

; List
(define nth (lst n)
  (if (= n 0)
      (first lst)
    (nth (rest lst) (- n 1))))

(define nthtail (lst n)
  (if (= n 0)
      lst
    (nthtail (rest lst) (- n 1))))

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

; Start
(in-package "User")
