void sema_stmt_return(SemaModule *sema, AstReturn *ret) {
	if (ret->has_value) {
		SemaType *type = sema_ast_expr_type(sema, &ret->expr, sema->returning);
		if (!type) {
			return;
		}
		if (!sema_types_equals(sema->returning, type)) {
			sema_err("expected to return `{sema::type}` got `{sema::type}`", sema->returning, type);
			return;
		}
	} else {
		if (!sema_types_equals(sema->returning, &primitives[PRIMITIVE_VOID])) {
			sema_err("expected to return `{sema::type}` but got nothing");
			return;
		}
	}
}
