#lang racket
(require "helpers.rkt")
(provide string->row)

(define (string->row str)
  (define (iter chars sub-str quoted?)
    (if (empty? chars)
      (list (list->string sub-str))
      (let ([char (car chars)])
        (cond [(equal? char #\") 
               (if quoted?
                 (iter (cdr chars) sub-str #f)
                 (iter (cdr chars) sub-str #t))]
              [(equal? char #\,)
               (if quoted?
                 (iter (cdr chars) (list-add sub-str char) quoted?)
                 (append (list (list->string sub-str)) 
                         (iter (cdr chars) '() quoted?)))]
               [else (iter (cdr chars) (list-add sub-str char) quoted?)]
               ))))
               (iter (string->list str) '() #f))
