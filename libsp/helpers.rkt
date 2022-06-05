#lang racket
(provide show
         list-add
         show-lines
         )

(define (show . var-args)
  (define (show-list args)
    (if (or (null? args) (empty? args))
      (newline)
      (begin (display (car args)) 
             (show-list (cdr args)))
      )
    )
  (show-list var-args)
  )

(define (list-add lst item)
  (append lst (list item)))

(define (show-lines lines)
  [if (null? lines)
    #t
    [begin (displayln (car lines))
           (show-lines (cdr lines))]])






