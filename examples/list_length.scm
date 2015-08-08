(define list-length (l)
  (if (nil? l)
      0
    (+ 1 (list-length (cdr l)))))

(set l (cons 1 (cons 2 (cons 3 (cons 4 (make-nil))))))

(list-length l)
