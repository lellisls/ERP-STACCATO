START TRANSACTION;
SET AUTOCOMMIT = 0;
INSERT INTO `mydb`.`Fornecedor` VALUES (1, 'Portinari Revestimentos', 'Portinari', '000.000.000/000-00', '000.000.000.000', 'Leandro da Silva', '123.456.123-10', 'Leandro', '11.111.111-6', '(11)0123-4567', '(11)0123-4567', '(11)0123-4567', NULL, NULL, 'teste@portinari.com.br', NULL, NULL, NULL, 0);
INSERT INTO `mydb`.`Fornecedor` VALUES (2, 'Apavisa Revestimentos', 'Apavisa', '000.000.000/000-00', '000.000.000.000', 'Leandro da Silva', '123.456.123-10', 'Leandro', '11.111.111-6', '(11)0123-4567', '(11)0123-4567', '(11)0123-4567', NULL, NULL, 'teste@apavisa.com.br', NULL, NULL, NULL, 0);
SELECT * FROM mydb.Produto;
 -- un, unUpd, colecao, colecaoUpd, tipo, tipoUpd, m2cx, m2cxUpd, pccx, pccxUpd, kgcx, kgcxUpd, formComercial, formComercialUpd, codComercial, codComercialUpd, codIndustrial, codIndustrialUpd, codBarras, codBarrasUpd, ncm, ncmUpd, cfop, cfopUpd, icms, icmsUpd, situacaoTributaria, situacaoTributariaUpd, qtdPallet, qtdPalletUpd, custo, custoUpd, ipi, ipiUpd, st, stUpd, precoVenda, precoVendaUpd, comissao, comissaoUpd, observacoes, observacoesUpd, origem, origemUpd, temLote, temLoteUpd, ui, uiUpd, validade, validadeUpd, expirado, expiradoUpd, descontinuado, descontinuadoUpd
COMMIT;
