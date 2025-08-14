$$
\begin{align}
    [\text{prog}] &\to [\text{Stmt}]^*
    \\
    [\text{stmt}] &\to
    \begin{cases}
        \text{exit}([\text{Expr}]);
        \\
        \text{let}\space\text{iden} = 
        [\text{Expr}];
    \end{cases}
    \\
    [\text{Expr}] &\to 
    \begin{cases}
        [\text{Term}]
        \\
        [\text{BinExpr}]
    \end{cases}
    \\
    [\text{Term}] &\to
    \begin{cases}
        \text{int\_lit}
        \\
        \text{iden}
        \\
        ([\text{Expr}])
    \end{cases}
    \\
    [\text{BinExpr}] &\to
    \begin{cases}
        [\text{Expr}] * [\text{Expr}] ,\space\text{prec} = 2
        \\
        [\text{Expr}] + [\text{Expr}] ,\space\text{prec} = 1
        \\
        [\text{Expr}] - [\text{Expr}] ,\space\text{prec} = 1
        \\
        [\text{Expr}] / [\text{Expr}] ,\space\text{prec} = 2
    \end{cases}
\end{align}
$$