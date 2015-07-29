(set odd? 0)

(set even? (lambda (n)
  (if (= n 0)
      1
    (odd? (pred n)))))

(set odd? (lambda (n)
  (if (= n 0)
      0
    (even? (pred n)))))

(even? 5)
(even? 10)
(odd? 5)
(odd? 10)
