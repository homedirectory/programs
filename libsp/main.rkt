#lang racket
(require "input.rkt"
         "data.rkt"
         "helpers.rkt"
         "csv.rkt"
         )

(define (dispatch msg)
  (cond [(equal? msg "find")
         ; replace data by (string-join data)
         (lambda (data . args) 
           (let ([found (apply find-book-by-name (cons (string-join data) args))])
             [if (empty? found)
               (show "Nothing found.")
               (show-lines found)]
         ))]
        [else (error "Unknown msg: " msg)])
  )

; input is handled based on the following rules:
; first word (token) is a message (e.g. find, add, remove)
; rest of the tokens are data -- input to the dispatched procedure
(define (handle-input input head books)
  (let* ([tokens (string-split input)]
         [msg (car tokens)]
         [data (cdr tokens)])
    ((dispatch msg) data head books)
    )
  )

; head - list of attribute names
; book - list of book attributes
(define (book->str head book)
  (map [lambda (name val)
         (string-append name ": " val)]
       head book)
  )

(define (show-book head book)
  (show-lines (book->str head book)))


; head - list of attribute names
; books - list of books (book is a list of attributes)
(define (repl head books)
  (let ([input (read-input)])
    (if (valid-input? input)
      (handle-input input head books)
      (begin (invalid-in-msg) (repl)))
    )
  )

(let* ([data (read-booklist)]
       [head (string->row (car data))]
       [books (map string->row (cdr data))])
  (show (length head) " columns:")
  (show "Header: " head)
  (show "Found " (length books) " books")
  ;(show-lines (map [lambda (book) (book->str head book)]
  ;                 books))
  (repl head books)
  '-------------done-------------
  )
