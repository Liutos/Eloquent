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
          (name-lambda ,name ,pars ,@body))
    (set-function-name! ,name ',name)))

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

(define reduce (list fn)
  (cond ((null? list)
         (signal "Parameter `list' can't be an empty list."))
        ((null? (rest list)) (first list))
        (else
         (fn (first list) (reduce (rest list) fn)))))

; Arithmetic Operations
;; +
(define bin+ (n m)
  (cond ((and2 (fixnum? n) (fixnum? m)) (fx+ n m))
        ((and2 (fixnum? n) (float? m)) (fp+ (fx->fp n) m))
        ((and2 (float? n) (fixnum? m)) (fp+ n (fx->fp m)))
        (else (fp+ n m))))

(define + ns
  (cond ((null? ns) 0)
        (else (reduce ns bin+))))

;; -
(define bin- (n m)
  (cond ((and2 (fixnum? n) (fixnum? m)) (fx- n m))
        ((and2 (fixnum? n) (float? m)) (fp- (fx->fp n) m))
        ((and2 (float? n) (fixnum? m)) (fp- n (fx->fp m)))
        (else (fp- n m))))

(define - (n . ns)
  (cond ((null? ns) (bin- 0 n))
        (else (bin- n (reduce ns bin+)))))

;; *
(define bin* (n m)
  (cond ((and2 (fixnum? n) (fixnum? m)) (fx* n m))
        ((and2 (fixnum? n) (float? m)) (fp* (fx->fp n) m))
        ((and2 (float? n) (fixnum? m)) (fp* n (fx->fp m)))
        (else (fp* n m))))

(define * ns
  (cond ((null? ns) 1)
        (else (reduce ns bin*))))

;; /
(define bin/ (n m)
  (cond ((and2 (fixnum? n) (fixnum? m)) (fx/ n m))
        ((and2 (fixnum? n) (float? m)) (fp/ (fx->fp n) m))
        ((and2 (float? n) (fixnum? m)) (fp/ n (fx->fp m)))
        (else (fp/ n m))))

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

; Unix CLI Tools
;; cat
(define cat (file)
  (let ((c (read-char file)))
    (if (eof? c)
        #t
      (begin
        (write-char c *standard-output*)
        (cat file)))))
