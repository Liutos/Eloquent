(set plus (lambda (a b)
            (if (= b 0)
                a
              (plus (succ a) (pred b)))))
(plus 10000 10000)
