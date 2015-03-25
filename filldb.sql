START TRANSACTION;
SET AUTOCOMMIT = 0;
INSERT INTO `mydb`.`Fornecedor` (`idFornecedor`, `razaoSocial`, `nomeFantasia`, `cnpj`, `inscEstadual`, `contatoNome`, `contatoCPF`, `contatoApelido`, `contatoRG`, `tel`, `telCel`, `telCom`, `idNextel`, `nextel`, `email`, `idUsuarioRel`, `idCadastroRel`, `idProfissionalRel`, `incompleto`) 
VALUES (1, 'Portinari Revestimentos', 'Portinari', '000.000.000/000-00', '000.000.000.000', 'Leandro da Silva', '123.456.123-10', 'Leandro', '11.111.111-6', '(11)0123-4567', '(11)0123-4567', '(11)0123-4567', NULL, NULL, 'teste@portinari.com.br', NULL, NULL, NULL, 0);
INSERT INTO `mydb`.`Fornecedor` (`idFornecedor`, `razaoSocial`, `nomeFantasia`, `cnpj`, `inscEstadual`, `contatoNome`, `contatoCPF`, `contatoApelido`, `contatoRG`, `tel`, `telCel`, `telCom`, `idNextel`, `nextel`, `email`, `idUsuarioRel`, `idCadastroRel`, `idProfissionalRel`, `incompleto`) 
VALUES (2, 'Apavisa Revestimentos', 'Apavisa', '000.000.000/000-00', '000.000.000.000', 'Leandro da Silva', '123.456.123-10', 'Leandro', '11.111.111-6', '(11)0123-4567', '(11)0123-4567', '(11)0123-4567', NULL, NULL, 'teste@apavisa.com.br', NULL, NULL, NULL, 0);
INSERT INTO `mydb`.`Produto` (`idProduto`, `idFornecedor`, `fornecedor`, `descricao`, `estoque`, `un`, `colecao`, `tipo`, `m2cx`, `pccx`, `kgcx`, `formComercial`, `codComercial`, `codIndustrial`, `codBarras`, `ncm`, `cfop`, `icms`, `situacaoTributaria`, `qtdPallet`, `custo`, `ipi`, `st`, `markup`, `precoVenda`, `comissao`, `observacoes`, `origem`, `descontinuado`, `temLote`, `ui`, `validade`, `expirado`) 
			          VALUES (1, 1, 'Portinari', 'Meu produto', '10', 'pccx', 'Colecao 01', '1', '11', '11', '22', '1X1X1', '012345', '1', '11111111', '11111111', '1234', '0', '1234', '66', '10' , '10', '0', '0', '100', '20', 'Exemplo de produto', '0' , '0', '1', '1', '2018-10-10', '0');
INSERT INTO `Venda` VALUES ('LJ01-150001',1,1,1,3,3,'2015-03-24 23:37:29',11000,11000,440,0,0,11440,7,'ABERTO');
INSERT INTO `Venda_has_Produto` VALUES ('LJ01-150001',1,0,1,'Portinari','Meu produto','',100,10,110,'pccx',11,11000,0,11000,0,11000,'PENDENTE');
COMMIT;
