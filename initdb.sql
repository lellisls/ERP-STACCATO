-- MySQL Script generated by MySQL Workbench
-- Sun 08 Mar 2015 11:09:40 PM BRT
-- Model: New Model    Version: 1.0
SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL,ALLOW_INVALID_DATES';

-- -----------------------------------------------------
-- Schema mydb
-- -----------------------------------------------------
DROP SCHEMA IF EXISTS `mydb` ;
CREATE SCHEMA IF NOT EXISTS `mydb` DEFAULT CHARACTER SET utf8 ;
USE `mydb` ;

-- -----------------------------------------------------
-- Table `mydb`.`Loja`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Loja` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Loja` (
  `idLoja` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `idEndereco` INT UNSIGNED NOT NULL,
  `descricao` VARCHAR(45) NOT NULL,
  `nomeFantasia` VARCHAR(45) NOT NULL,
  `razaoSocial` VARCHAR(60) NOT NULL,
  `tel` VARCHAR(18) NOT NULL,
  `inscEstadual` VARCHAR(20) NOT NULL,
  `sigla` VARCHAR(4) NOT NULL,
  `cnpj` VARCHAR(18) NOT NULL,
  `codUF` INT NOT NULL DEFAULT 0,
  `porcentagemFrete` INT NOT NULL DEFAULT 4,
  `valorMinimoFrete` INT NOT NULL DEFAULT 80,
  PRIMARY KEY (`idLoja`),
  UNIQUE INDEX `idLoja_UNIQUE` (`idLoja` ASC),
  FULLTEXT INDEX `idxFT` (`descricao` ASC, `nomeFantasia` ASC, `razaoSocial` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Usuario`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Usuario` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Usuario` (
  `idUsuario` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `idLoja` INT UNSIGNED NOT NULL,
  `user` VARCHAR(20) NOT NULL,
  `passwd` VARCHAR(42) NOT NULL,
  `tipo` VARCHAR(45) NOT NULL,
  `nome` VARCHAR(45) NOT NULL,
  `sigla` VARCHAR(3) NOT NULL,
  PRIMARY KEY (`idUsuario`),
  UNIQUE INDEX `sigla_UNIQUE` (`sigla` ASC),
  UNIQUE INDEX `user_UNIQUE` (`user` ASC),
  INDEX `fk_Usuario_Loja1_idx` (`idLoja` ASC),
  FULLTEXT INDEX `idxFT` (`nome` ASC, `tipo` ASC),
  CONSTRAINT `fk_Usuario_Loja1`
    FOREIGN KEY (`idLoja`)
    REFERENCES `mydb`.`Loja` (`idLoja`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Profissional`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Profissional` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Profissional` (
  `idProfissional` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `nome` VARCHAR(45) NOT NULL,
  `tipo` VARCHAR(45) NULL,
  `tel` VARCHAR(45) NULL,
  `email` VARCHAR(45) NULL,
  `banco` VARCHAR(45) NULL,
  `agencia` VARCHAR(45) NULL,
  `cc` VARCHAR(45) NULL,
  `nomeBanco` VARCHAR(45) NULL,
  `cpfBanco` VARCHAR(45) NULL,
  PRIMARY KEY (`idProfissional`),
  FULLTEXT INDEX `idxFT` (`nome` ASC, `tipo` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Cadastro`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Cadastro` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Cadastro` (
  `idCadastro` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `pfpj` VARCHAR(45) NOT NULL,
  `clienteFornecedor` VARCHAR(45) NOT NULL,
  `nome_razao` VARCHAR(45) NOT NULL,
  `nomeFantasia` VARCHAR(45) NULL,
  `cpf` VARCHAR(14) NULL,
  `cnpj` VARCHAR(18) NULL,
  `rg` VARCHAR(12) NULL,
  `inscEstadual` VARCHAR(45) NULL,
  `contatoNome` VARCHAR(45) NULL,
  `contatoCPF` VARCHAR(45) NULL,
  `contatoApelido` VARCHAR(45) NULL,
  `contatoRG` VARCHAR(45) NULL,
  `tel` VARCHAR(18) NULL,
  `telCel` VARCHAR(18) NULL,
  `telCom` VARCHAR(18) NULL,
  `idNextel` VARCHAR(16) NULL,
  `nextel` VARCHAR(18) NULL,
  `email` VARCHAR(45) NULL,
  `idUsuarioRel` INT UNSIGNED NULL,
  `idCadastroRel` INT UNSIGNED NULL,
  `idProfissionalRel` INT UNSIGNED NULL,
  `incompleto` TINYINT(1) NULL,
  PRIMARY KEY (`idCadastro`),
  INDEX `fk_Cadastro_Usuario1_idx` (`idUsuarioRel` ASC),
  INDEX `fk_Cadastro_Cadastro1_idx` (`idCadastroRel` ASC),
  INDEX `fk_Cadastro_Profissional1_idx` (`idProfissionalRel` ASC),
  FULLTEXT INDEX `idxFT` (`nome_razao` ASC, `nomeFantasia` ASC, `cpf` ASC, `cnpj` ASC),
  INDEX `fk_Cadastro_Produto_idx` (`idCadastro` ASC),
  CONSTRAINT `fk_Cadastro_Usuario1`
    FOREIGN KEY (`idUsuarioRel`)
    REFERENCES `mydb`.`Usuario` (`idUsuario`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Cadastro_Cadastro1`
    FOREIGN KEY (`idCadastroRel`)
    REFERENCES `mydb`.`Cadastro` (`idCadastro`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Cadastro_Profissional1`
    FOREIGN KEY (`idProfissionalRel`)
    REFERENCES `mydb`.`Profissional` (`idProfissional`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Produto`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Produto` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Produto` (
  `idProduto` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `idFornecedor` INT UNSIGNED NOT NULL,
  `fornecedor` VARCHAR(45) NOT NULL,
  `descricao` VARCHAR(45) NOT NULL,
  `estoque` INT NOT NULL DEFAULT 0,
  `un` VARCHAR(10) NOT NULL,
  `colecao` VARCHAR(45) NULL,
  `tipo` VARCHAR(45) NULL,
  `m2cx` DOUBLE NULL,
  `pccx` DOUBLE NULL,
  `formComercial` VARCHAR(45) NULL,
  `codComercial` VARCHAR(45) NOT NULL,
  `codIndustrial` VARCHAR(45) NULL,
  `codBarras` VARCHAR(45) NULL,
  `ncm` VARCHAR(45) NOT NULL,
  `cfop` VARCHAR(45) NULL,
  `icms` VARCHAR(45) NULL,
  `situacaoTributaria` INT NULL,
  `qtdPallet` DOUBLE NULL,
  `custo` DOUBLE NOT NULL,
  `ipi` DOUBLE NULL,
  `st` DOUBLE NULL,
  `markup` DOUBLE NULL,
  `precoVenda` DOUBLE NULL,
  `comissao` DOUBLE NULL,
  `observacoes` VARCHAR(800) NULL,
  `origem` INT NULL,
  `descontinuado` VARCHAR(5) NOT NULL DEFAULT 'NÃO',
  `temLote` VARCHAR(5) NULL DEFAULT 'NÃO',
  `ui` INT NOT NULL DEFAULT 0,
  UNIQUE INDEX `idProduto_UNIQUE` (`idProduto` ASC),
  PRIMARY KEY (`idProduto`),
  FULLTEXT INDEX `idxFT` (`fornecedor` ASC, `descricao` ASC, `colecao` ASC, `codComercial` ASC),
  INDEX `fk_Produto_Cadastro1_idx` (`idFornecedor` ASC),
  CONSTRAINT `fk_Produto_Cadastro1`
    FOREIGN KEY (`idFornecedor`)
    REFERENCES `mydb`.`Cadastro` (`idCadastro`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
COMMENT = '													';


-- -----------------------------------------------------
-- Table `mydb`.`Transportadora`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Transportadora` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Transportadora` (
  `idTransportadora` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `idEndereco` INT UNSIGNED NOT NULL,
  `razaoSocial` VARCHAR(60) NOT NULL,
  `nomeFantasia` VARCHAR(45) NULL,
  `cnpj` VARCHAR(18) NOT NULL,
  `inscEstadual` VARCHAR(20) NOT NULL,
  `placaVeiculo` VARCHAR(8) NULL,
  `antt` VARCHAR(10) NULL,
  `tel` VARCHAR(18) NULL,
  PRIMARY KEY (`idTransportadora`),
  FULLTEXT INDEX `idxFT` (`razaoSocial` ASC, `nomeFantasia` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Orcamento`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Orcamento` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Orcamento` (
  `idOrcamento` VARCHAR(20) NOT NULL,
  `idLoja` INT UNSIGNED NOT NULL,
  `idUsuario` INT UNSIGNED NOT NULL,
  `idCadastroCliente` INT UNSIGNED NOT NULL,
  `idEnderecoEntrega` INT UNSIGNED NULL,
  `idProfissional` INT UNSIGNED NOT NULL,
  `data` DATETIME NOT NULL,
  `total` DOUBLE NOT NULL,
  `desconto` DOUBLE NOT NULL DEFAULT 0,
  `frete` DOUBLE NOT NULL DEFAULT 0,
  `validade` INT NOT NULL DEFAULT 7,
  `status` VARCHAR(45) NOT NULL DEFAULT 'ATIVO',
  `motivoCancelamento` VARCHAR(45) NULL,
  PRIMARY KEY (`idOrcamento`, `idLoja`),
  INDEX `fk_Orcamento_Cadastro1_idx` (`idCadastroCliente` ASC),
  INDEX `fk_Orcamento_Loja1_idx` (`idLoja` ASC),
  INDEX `fk_Orcamento_Usuario1_idx` (`idUsuario` ASC),
  UNIQUE INDEX `idOrcamento_UNIQUE` (`idOrcamento` ASC),
  INDEX `fk_Orcamento_Profissional1_idx` (`idProfissional` ASC),
  CONSTRAINT `fk_Orcamento_Cadastro1`
    FOREIGN KEY (`idCadastroCliente`)
    REFERENCES `mydb`.`Cadastro` (`idCadastro`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_Loja1`
    FOREIGN KEY (`idLoja`)
    REFERENCES `mydb`.`Loja` (`idLoja`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_Usuario1`
    FOREIGN KEY (`idUsuario`)
    REFERENCES `mydb`.`Usuario` (`idUsuario`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_Profissional1`
    FOREIGN KEY (`idProfissional`)
    REFERENCES `mydb`.`Profissional` (`idProfissional`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ContaAPagar`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ContaAPagar` ;

CREATE TABLE IF NOT EXISTS `mydb`.`ContaAPagar` (
  `idVenda` VARCHAR(20) NOT NULL,
  `dataEmissao` DATETIME NOT NULL,
  `dataPagamento` DATETIME NULL,
  `pago` VARCHAR(5) NOT NULL DEFAULT 'NÃO',
  `formaPagamento` VARCHAR(45) NULL,
  `idProduto` INT NULL,
  PRIMARY KEY (`idVenda`, `dataEmissao`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ContaAReceber`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ContaAReceber` ;

CREATE TABLE IF NOT EXISTS `mydb`.`ContaAReceber` (
  `pago` VARCHAR(5) NOT NULL DEFAULT 'NÃO',
  `dataEmissao` DATETIME NOT NULL,
  `dataPagamento` DATETIME NULL,
  `idVenda` VARCHAR(20) NOT NULL,
  PRIMARY KEY (`idVenda`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`VendaCancelada`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`VendaCancelada` ;

CREATE TABLE IF NOT EXISTS `mydb`.`VendaCancelada` (
  `descricao` VARCHAR(180) NULL,
  `dataCancelamento` DATETIME NULL)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`PedidoTransportadora`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`PedidoTransportadora` ;

CREATE TABLE IF NOT EXISTS `mydb`.`PedidoTransportadora` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `idTransportadora` INT UNSIGNED NOT NULL,
  `idPedido` VARCHAR(20) NOT NULL,
  `tipo` VARCHAR(45) NOT NULL,
  `dataEmissao` DATETIME NOT NULL,
  `dataEntrega` DATETIME NULL,
  `status` VARCHAR(45) NULL,
  PRIMARY KEY (`id`, `idTransportadora`, `idPedido`, `tipo`),
  INDEX `fk_PedidoTransportadora_Transportadora1_idx` (`idTransportadora` ASC),
  UNIQUE INDEX `id_UNIQUE` (`id` ASC),
  CONSTRAINT `fk_PedidoTransportadora_Transportadora1`
    FOREIGN KEY (`idTransportadora`)
    REFERENCES `mydb`.`Transportadora` (`idTransportadora`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Orcamento_has_Produto`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Orcamento_has_Produto` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Orcamento_has_Produto` (
  `idOrcamento` VARCHAR(20) NOT NULL,
  `idLoja` INT UNSIGNED NOT NULL,
  `item` INT NOT NULL,
  `idProduto` INT UNSIGNED NOT NULL,
  `fornecedor` VARCHAR(45) NOT NULL,
  `produto` VARCHAR(45) NULL,
  `obs` VARCHAR(45) NOT NULL,
  `prcUnitario` DOUBLE NOT NULL,
  `caixas` INT NOT NULL,
  `qte` DOUBLE NOT NULL,
  `un` VARCHAR(45) NOT NULL,
  `unCaixa` DOUBLE NOT NULL,
  `parcial` DOUBLE NOT NULL,
  `desconto` DOUBLE NOT NULL DEFAULT 0,
  `parcialDesc` DOUBLE NOT NULL,
  `descGlobal` DOUBLE NOT NULL,
  `total` DOUBLE NOT NULL,
  PRIMARY KEY (`idOrcamento`, `idLoja`, `item`, `idProduto`),
  INDEX `fk_Orcamento_has_Produto_Produto1_idx` (`idProduto` ASC),
  INDEX `fk_Orcamento_has_Produto_Orcamento1_idx` (`idOrcamento` ASC, `idLoja` ASC),
  CONSTRAINT `fk_Orcamento_has_Produto_Produto1`
    FOREIGN KEY (`idProduto`)
    REFERENCES `mydb`.`Produto` (`idProduto`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_has_Produto_Orcamento1`
    FOREIGN KEY (`idOrcamento` , `idLoja`)
    REFERENCES `mydb`.`Orcamento` (`idOrcamento` , `idLoja`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Venda`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Venda` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Venda` (
  `idVenda` VARCHAR(20) NOT NULL,
  `idLoja` INT UNSIGNED NOT NULL,
  `idUsuario` INT UNSIGNED NOT NULL,
  `idCadastroCliente` INT UNSIGNED NOT NULL,
  `idEnderecoEntrega` INT UNSIGNED NOT NULL,
  `idProfissional` INT UNSIGNED NOT NULL,
  `data` DATETIME NOT NULL,
  `total` DOUBLE NOT NULL,
  `desconto` DOUBLE NOT NULL DEFAULT 0,
  `frete` DOUBLE NOT NULL DEFAULT 0,
  `validade` INT NOT NULL DEFAULT 7,
  `status` VARCHAR(45) NOT NULL DEFAULT 'ATIVO',
  PRIMARY KEY (`idVenda`, `idLoja`),
  INDEX `fk_Orcamento_Cadastro1_idx` (`idCadastroCliente` ASC),
  INDEX `fk_Orcamento_Loja1_idx` (`idLoja` ASC),
  INDEX `fk_Orcamento_Usuario1_idx` (`idUsuario` ASC),
  UNIQUE INDEX `idOrcamento_UNIQUE` (`idVenda` ASC),
  INDEX `fk_Orcamento_Profissional1_idx` (`idProfissional` ASC),
  CONSTRAINT `fk_Orcamento_Cadastro11`
    FOREIGN KEY (`idCadastroCliente`)
    REFERENCES `mydb`.`Cadastro` (`idCadastro`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_Loja11`
    FOREIGN KEY (`idLoja`)
    REFERENCES `mydb`.`Loja` (`idLoja`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_Usuario11`
    FOREIGN KEY (`idUsuario`)
    REFERENCES `mydb`.`Usuario` (`idUsuario`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_Profissional11`
    FOREIGN KEY (`idProfissional`)
    REFERENCES `mydb`.`Profissional` (`idProfissional`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Venda_has_Produto`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Venda_has_Produto` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Venda_has_Produto` (
  `idVenda` VARCHAR(20) NOT NULL,
  `idLoja` INT UNSIGNED NOT NULL,
  `item` INT NOT NULL,
  `idProduto` INT UNSIGNED NOT NULL,
  `fornecedor` VARCHAR(45) NOT NULL,
  `produto` VARCHAR(45) NULL,
  `obs` VARCHAR(45) NOT NULL,
  `prcUnitario` DOUBLE NOT NULL,
  `caixas` INT NOT NULL,
  `qte` DOUBLE NOT NULL,
  `un` VARCHAR(45) NOT NULL,
  `unCaixa` DOUBLE NOT NULL,
  `parcial` DOUBLE NOT NULL,
  `desconto` DOUBLE NOT NULL DEFAULT 0,
  `parcialDesc` DOUBLE NOT NULL,
  `descGlobal` DOUBLE NOT NULL,
  `total` DOUBLE NOT NULL,
  `status` VARCHAR(45) NULL,
  PRIMARY KEY (`idVenda`, `idLoja`, `item`, `idProduto`),
  INDEX `fk_Orcamento_has_Produto_Produto1_idx` (`idProduto` ASC),
  INDEX `fk_Orcamento_has_Produto_Orcamento1_idx` (`idVenda` ASC, `idLoja` ASC),
  CONSTRAINT `fk_Orcamento_has_Produto_Produto11`
    FOREIGN KEY (`idProduto`)
    REFERENCES `mydb`.`Produto` (`idProduto`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_has_Produto_Orcamento11`
    FOREIGN KEY (`idVenda` , `idLoja`)
    REFERENCES `mydb`.`Venda` (`idVenda` , `idLoja`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`PedidoFornecedor_has_Produto`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`PedidoFornecedor_has_Produto` ;

CREATE TABLE IF NOT EXISTS `mydb`.`PedidoFornecedor_has_Produto` (
  `idPedido` VARCHAR(20) NOT NULL,
  `idLoja` INT UNSIGNED NOT NULL,
  `item` INT NOT NULL,
  `idProduto` INT UNSIGNED NOT NULL,
  `fornecedor` VARCHAR(45) NOT NULL,
  `produto` VARCHAR(45) NULL,
  `obs` VARCHAR(45) NOT NULL,
  `prcUnitario` DOUBLE NOT NULL,
  `caixas` INT NOT NULL,
  `qte` DOUBLE NOT NULL,
  `un` VARCHAR(45) NOT NULL,
  `unCaixa` DOUBLE NOT NULL,
  `parcial` DOUBLE NOT NULL,
  `desconto` DOUBLE NOT NULL DEFAULT 0,
  `parcialDesc` DOUBLE NOT NULL,
  `descGlobal` DOUBLE NOT NULL,
  `total` DOUBLE NOT NULL,
  `status` VARCHAR(45) NOT NULL DEFAULT 'PENDENTE',
  PRIMARY KEY (`idPedido`, `idLoja`, `item`, `idProduto`),
  INDEX `fk_Orcamento_has_Produto_Produto1_idx` (`idProduto` ASC),
  INDEX `fk_Orcamento_has_Produto_Orcamento1_idx` (`idPedido` ASC, `idLoja` ASC),
  CONSTRAINT `fk_Orcamento_has_Produto_Produto110`
    FOREIGN KEY (`idProduto`)
    REFERENCES `mydb`.`Produto` (`idProduto`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_has_Produto_Orcamento110`
    FOREIGN KEY (`idPedido` , `idLoja`)
    REFERENCES `mydb`.`Venda` (`idVenda` , `idLoja`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`PedidoFornecedor`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`PedidoFornecedor` ;

CREATE TABLE IF NOT EXISTS `mydb`.`PedidoFornecedor` (
  `idPedido` VARCHAR(20) NOT NULL,
  `idLoja` INT UNSIGNED NOT NULL,
  `idUsuario` INT UNSIGNED NOT NULL,
  `idCadastroCliente` INT UNSIGNED NOT NULL,
  `idEnderecoEntrega` INT UNSIGNED NOT NULL,
  `idProfissional` INT UNSIGNED NOT NULL,
  `data` DATETIME NOT NULL,
  `total` DOUBLE NOT NULL,
  `desconto` DOUBLE NOT NULL DEFAULT 0,
  `frete` DOUBLE NOT NULL DEFAULT 0,
  `validade` INT NOT NULL DEFAULT 7,
  `status` VARCHAR(45) NOT NULL DEFAULT 'ATIVO',
  `entregue` VARCHAR(5) NOT NULL DEFAULT 'NÃO',
  PRIMARY KEY (`idPedido`, `idLoja`),
  INDEX `fk_Orcamento_Cadastro1_idx` (`idCadastroCliente` ASC),
  INDEX `fk_Orcamento_Loja1_idx` (`idLoja` ASC),
  INDEX `fk_Orcamento_Usuario1_idx` (`idUsuario` ASC),
  UNIQUE INDEX `idOrcamento_UNIQUE` (`idPedido` ASC),
  INDEX `fk_Orcamento_Profissional1_idx` (`idProfissional` ASC),
  CONSTRAINT `fk_Orcamento_Cadastro110`
    FOREIGN KEY (`idCadastroCliente`)
    REFERENCES `mydb`.`Cadastro` (`idCadastro`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_Loja110`
    FOREIGN KEY (`idLoja`)
    REFERENCES `mydb`.`Loja` (`idLoja`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_Usuario110`
    FOREIGN KEY (`idUsuario`)
    REFERENCES `mydb`.`Usuario` (`idUsuario`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Orcamento_Profissional110`
    FOREIGN KEY (`idProfissional`)
    REFERENCES `mydb`.`Profissional` (`idProfissional`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`NFe`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`NFe` ;

CREATE TABLE IF NOT EXISTS `mydb`.`NFe` (
  `idNFe` INT NOT NULL AUTO_INCREMENT,
  `NFe` MEDIUMTEXT NULL,
  `idVenda` VARCHAR(20) NOT NULL,
  `idLoja` INT UNSIGNED NOT NULL,
  `status` VARCHAR(45) NOT NULL DEFAULT 'PENDENTE',
  `chaveAcesso` VARCHAR(50) NOT NULL,
  PRIMARY KEY (`idNFe`, `idVenda`, `idLoja`),
  INDEX `fk_NFe_Venda1_idx` (`idVenda` ASC, `idLoja` ASC),
  CONSTRAINT `fk_NFe_Venda1`
    FOREIGN KEY (`idVenda` , `idLoja`)
    REFERENCES `mydb`.`Venda` (`idVenda` , `idLoja`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ContaAPagar_has_Produto`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ContaAPagar_has_Produto` ;

CREATE TABLE IF NOT EXISTS `mydb`.`ContaAPagar_has_Produto` (
  `idVenda` VARCHAR(20) NOT NULL,
  `idProduto` INT UNSIGNED NOT NULL,
  `produto` VARCHAR(45) NULL,
  `prcUnitario` DOUBLE NULL,
  `qte` DOUBLE NULL,
  `un` VARCHAR(45) NULL,
  `total` DOUBLE NULL,
  PRIMARY KEY (`idVenda`, `idProduto`),
  INDEX `fk_ContaAPagar_has_Produto_Produto1_idx` (`idProduto` ASC),
  INDEX `fk_ContaAPagar_has_Produto_ContaAPagar1_idx` (`idVenda` ASC),
  CONSTRAINT `fk_ContaAPagar_has_Produto_ContaAPagar1`
    FOREIGN KEY (`idVenda`)
    REFERENCES `mydb`.`ContaAPagar` (`idVenda`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_ContaAPagar_has_Produto_Produto1`
    FOREIGN KEY (`idProduto`)
    REFERENCES `mydb`.`Produto` (`idProduto`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ContaAPagar_has_Pagamento`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ContaAPagar_has_Pagamento` ;

CREATE TABLE IF NOT EXISTS `mydb`.`ContaAPagar_has_Pagamento` (
  `ContaAPagar_idVenda` VARCHAR(20) NOT NULL,
  `ContaAPagar_dataEmissao` DATETIME NOT NULL,
  `Pagamento_idPagamento` INT NOT NULL,
  PRIMARY KEY (`ContaAPagar_idVenda`, `ContaAPagar_dataEmissao`, `Pagamento_idPagamento`),
  INDEX `fk_ContaAPagar_has_Pagamento_ContaAPagar1_idx` (`ContaAPagar_idVenda` ASC, `ContaAPagar_dataEmissao` ASC),
  CONSTRAINT `fk_ContaAPagar_has_Pagamento_ContaAPagar1`
    FOREIGN KEY (`ContaAPagar_idVenda` , `ContaAPagar_dataEmissao`)
    REFERENCES `mydb`.`ContaAPagar` (`idVenda` , `dataEmissao`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ContaAReceber_has_Pagamento`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ContaAReceber_has_Pagamento` ;

CREATE TABLE IF NOT EXISTS `mydb`.`ContaAReceber_has_Pagamento` (
  `ContaAReceber_idVenda` VARCHAR(20) NOT NULL,
  `Pagamento_idPagamento` INT NOT NULL,
  PRIMARY KEY (`ContaAReceber_idVenda`, `Pagamento_idPagamento`),
  INDEX `fk_ContaAReceber_has_Pagamento_ContaAReceber1_idx` (`ContaAReceber_idVenda` ASC),
  CONSTRAINT `fk_ContaAReceber_has_Pagamento_ContaAReceber1`
    FOREIGN KEY (`ContaAReceber_idVenda`)
    REFERENCES `mydb`.`ContaAReceber` (`idVenda`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Venda_has_Pagamento`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Venda_has_Pagamento` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Venda_has_Pagamento` (
  `idPagamento` INT NOT NULL AUTO_INCREMENT,
  `idVenda` VARCHAR(20) NOT NULL,
  `idLoja` INT UNSIGNED NOT NULL,
  `tipo` VARCHAR(45) NOT NULL,
  `parcela` INT NOT NULL,
  `valor` DOUBLE NOT NULL,
  `data` DATE NOT NULL,
  PRIMARY KEY (`idPagamento`, `idVenda`, `idLoja`),
  INDEX `fk_Venda_has_Pagamento_Venda1_idx` (`idVenda` ASC, `idLoja` ASC),
  CONSTRAINT `fk_Venda_has_Pagamento_Venda1`
    FOREIGN KEY (`idVenda` , `idLoja`)
    REFERENCES `mydb`.`Venda` (`idVenda` , `idLoja`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Alcadas`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Alcadas` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Alcadas` (
  `idAlcada` INT NOT NULL,
  `idLoja` INT UNSIGNED NOT NULL,
  `tipoFuncionario` VARCHAR(45) NOT NULL DEFAULT '',
  `debito` DOUBLE NOT NULL DEFAULT 0,
  `credito` DOUBLE NOT NULL DEFAULT 0,
  `cheque` DOUBLE NOT NULL DEFAULT 0,
  `dinheiro` DOUBLE NOT NULL DEFAULT 0,
  `boleto` DOUBLE NOT NULL DEFAULT 0,
  PRIMARY KEY (`idAlcada`, `idLoja`),
  UNIQUE INDEX `idAlcada_UNIQUE` (`idAlcada` ASC),
  INDEX `fk_Alçadas_Loja1_idx` (`idLoja` ASC),
  CONSTRAINT `fk_Alçadas_Loja1`
    FOREIGN KEY (`idLoja`)
    REFERENCES `mydb`.`Loja` (`idLoja`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Endereco`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Endereco` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Endereco` (
  `idEndereco` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `descricao` VARCHAR(45) NULL,
  `cep` VARCHAR(10) NULL,
  `logradouro` VARCHAR(90) NULL,
  `numero` VARCHAR(45) NULL,
  `complemento` VARCHAR(45) NULL,
  `bairro` VARCHAR(72) NULL,
  `cidade` VARCHAR(50) NULL,
  `uf` VARCHAR(45) NULL,
  `ativo` TINYINT(1) NOT NULL DEFAULT 1,
  `idCadastro` INT UNSIGNED NULL,
  PRIMARY KEY (`idEndereco`),
  INDEX `fk_Endereco_Cadastro1_idx` (`idCadastro` ASC),
  CONSTRAINT `fk_Endereco_Cadastro1`
    FOREIGN KEY (`idCadastro`)
    REFERENCES `mydb`.`Cadastro` (`idCadastro`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Preco`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Preco` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Preco` (
  `idPreco` INT NOT NULL,
  `idProduto` INT NULL,
  `preco` DOUBLE NULL,
  `validade` DATE NULL,
  PRIMARY KEY (`idPreco`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Produto_has_Preco`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Produto_has_Preco` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Produto_has_Preco` (
  `idProduto` INT UNSIGNED NOT NULL,
  `idPreco` INT NOT NULL,
  `preco` DOUBLE NULL,
  `validade` DATE NULL,
  PRIMARY KEY (`idProduto`, `idPreco`),
  INDEX `fk_Produto_has_Preco_Preco1_idx` (`idPreco` ASC),
  INDEX `fk_Produto_has_Preco_Produto1_idx` (`idProduto` ASC),
  CONSTRAINT `fk_Produto_has_Preco_Produto1`
    FOREIGN KEY (`idProduto`)
    REFERENCES `mydb`.`Produto` (`idProduto`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_Produto_has_Preco_Preco1`
    FOREIGN KEY (`idPreco`)
    REFERENCES `mydb`.`Preco` (`idPreco`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;

USE `mydb` ;

-- -----------------------------------------------------
-- Placeholder table for view `mydb`.`ViewOrcamento`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `mydb`.`ViewOrcamento` (`idUsuario` INT, `'Código'` INT, `'Cliente'` INT, `'Total'` INT, `'Data de emissão'` INT, `'Status'` INT, `'Dias restantes'` INT);

-- -----------------------------------------------------
-- Placeholder table for view `mydb`.`ViewVendas`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `mydb`.`ViewVendas` (`idVenda` INT, `idLoja` INT, `idUsuario` INT, `idCadastroCliente` INT, `idEnderecoEntrega` INT, `idProfissional` INT, `data` INT, `total` INT, `desconto` INT, `frete` INT, `validade` INT, `status` INT);

-- -----------------------------------------------------
-- View `mydb`.`ViewOrcamento`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `mydb`.`ViewOrcamento` ;
DROP TABLE IF EXISTS `mydb`.`ViewOrcamento`;
USE `mydb`;
CREATE OR REPLACE VIEW `ViewOrcamento` AS
    SELECT 
        idUsuario, idOrcamento AS 'Código' , nome_razao AS 'Cliente', total AS 'Total', data 'Data de emissão', status AS 'Status', validade - DATEDIFF( NOW(), data ) AS 'Dias restantes'
    FROM
        Cadastro
            RIGHT JOIN
        Orcamento ON idCadastroCliente = idCadastro;

-- -----------------------------------------------------
-- View `mydb`.`ViewVendas`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `mydb`.`ViewVendas` ;
DROP TABLE IF EXISTS `mydb`.`ViewVendas`;
USE `mydb`;
CREATE  OR REPLACE VIEW `ViewVendas` AS
    SELECT 
        *
    FROM
        `ViewOrcamento`
	NATURAL RIGHT JOIN 
        `Venda`;
    ;

SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

-- -----------------------------------------------------
-- Data for table `mydb`.`Loja`
-- -----------------------------------------------------
START TRANSACTION;
USE `mydb`;
INSERT INTO `mydb`.`Loja` (`idLoja`, `idEndereco`, `descricao`, `nomeFantasia`, `razaoSocial`, `tel`, `inscEstadual`, `sigla`, `cnpj`, `codUF`, `porcentagemFrete`, `valorMinimoFrete`) VALUES (1, 5, 'Loja 1', 'LOJA 01', 'LOJA 01 EPP', '(12)1234-5678', '000.000.000.000', 'LJ01', '19.261.391/0001-26', 35, 4, 80);

COMMIT;


-- -----------------------------------------------------
-- Data for table `mydb`.`Usuario`
-- -----------------------------------------------------
START TRANSACTION;
USE `mydb`;
INSERT INTO `mydb`.`Usuario` (`idUsuario`, `idLoja`, `user`, `passwd`, `tipo`, `nome`, `sigla`) VALUES (1, 1, 'admin', '*A4B6157319038724E3560894F7F932C8886EBFCF', 'ADMINISTRADOR', 'Administrador', 'ADM');
INSERT INTO `mydb`.`Usuario` (`idUsuario`, `idLoja`, `user`, `passwd`, `tipo`, `nome`, `sigla`) VALUES (2, 1, 'lellis', '*A4B6157319038724E3560894F7F932C8886EBFCF', 'VENDEDOR', 'Lucas S. Lellis', 'LSL');

COMMIT;


-- -----------------------------------------------------
-- Data for table `mydb`.`Profissional`
-- -----------------------------------------------------
START TRANSACTION;
USE `mydb`;
INSERT INTO `mydb`.`Profissional` (`idProfissional`, `nome`, `tipo`, `tel`, `email`, `banco`, `agencia`, `cc`, `nomeBanco`, `cpfBanco`) VALUES (1, 'Não há', '', '', '', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `mydb`.`Profissional` (`idProfissional`, `nome`, `tipo`, `tel`, `email`, `banco`, `agencia`, `cc`, `nomeBanco`, `cpfBanco`) VALUES (2, 'Daniel Melo Azevedo', 'ENGENHEIRO', '(15)8146-5647', 'daniel@engenheiro.com.br', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `mydb`.`Profissional` (`idProfissional`, `nome`, `tipo`, `tel`, `email`, `banco`, `agencia`, `cc`, `nomeBanco`, `cpfBanco`) VALUES (3, 'Mateus Cavalcanti Pinto', 'ARQUITETO', '(31)3772-8301', 'mateus.carvalho@gmail.com.br', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `mydb`.`Profissional` (`idProfissional`, `nome`, `tipo`, `tel`, `email`, `banco`, `agencia`, `cc`, `nomeBanco`, `cpfBanco`) VALUES (4, 'Vitória Barros Carvalho', 'ARQUITETO', '(21)2166-7146', 'contato@vitoria.arq', NULL, NULL, NULL, NULL, NULL);

COMMIT;


-- -----------------------------------------------------
-- Data for table `mydb`.`Cadastro`
-- -----------------------------------------------------
START TRANSACTION;
USE `mydb`;
INSERT INTO `mydb`.`Cadastro` (`idCadastro`, `pfpj`, `clienteFornecedor`, `nome_razao`, `nomeFantasia`, `cpf`, `cnpj`, `rg`, `inscEstadual`, `contatoNome`, `contatoCPF`, `contatoApelido`, `contatoRG`, `tel`, `telCel`, `telCom`, `idNextel`, `nextel`, `email`, `idUsuarioRel`, `idCadastroRel`, `idProfissionalRel`, `incompleto`) VALUES (1, 'PJ', 'AMBOS', 'Marcos Oliveira Ferreira', 'Bisa Massas', '432.432.477-80', '00.696.325/0001-50', '41.875.789-6', '920.048.586.200', NULL, NULL, NULL, NULL, '(11)2801-2013', '(11)2801-2013', '(11)2801-2013', '00*0000000*00000', '(11)2801-2013', 'LucasSousaRibeiro@teleworm.us ', NULL, NULL, NULL, 0);
INSERT INTO `mydb`.`Cadastro` (`idCadastro`, `pfpj`, `clienteFornecedor`, `nome_razao`, `nomeFantasia`, `cpf`, `cnpj`, `rg`, `inscEstadual`, `contatoNome`, `contatoCPF`, `contatoApelido`, `contatoRG`, `tel`, `telCel`, `telCom`, `idNextel`, `nextel`, `email`, `idUsuarioRel`, `idCadastroRel`, `idProfissionalRel`, `incompleto`) VALUES (2, 'PJ', 'FORNECEDOR', 'Portinari', 'Portinari', '000.000.000-80', '00.334.497/0001-84', '00.000.000-6', 'ISENTO', NULL, NULL, NULL, NULL, '(48)3431-6333', NULL, NULL, NULL, NULL, 'contato@ceramicaportinari.com.br', NULL, NULL, NULL, 0);
INSERT INTO `mydb`.`Cadastro` (`idCadastro`, `pfpj`, `clienteFornecedor`, `nome_razao`, `nomeFantasia`, `cpf`, `cnpj`, `rg`, `inscEstadual`, `contatoNome`, `contatoCPF`, `contatoApelido`, `contatoRG`, `tel`, `telCel`, `telCom`, `idNextel`, `nextel`, `email`, `idUsuarioRel`, `idCadastroRel`, `idProfissionalRel`, `incompleto`) VALUES (3, 'PJ', 'CLIENTE', 'Eduardo Pinto de Oliveira', NULL, '116.875.908-00', '11.111.111/1111-11', NULL, 'isento', NULL, NULL, NULL, NULL, '(11)4153-0948', NULL, NULL, NULL, NULL, 'edu.oliv@globo.com', 1, NULL, NULL, 0);
INSERT INTO `mydb`.`Cadastro` (`idCadastro`, `pfpj`, `clienteFornecedor`, `nome_razao`, `nomeFantasia`, `cpf`, `cnpj`, `rg`, `inscEstadual`, `contatoNome`, `contatoCPF`, `contatoApelido`, `contatoRG`, `tel`, `telCel`, `telCom`, `idNextel`, `nextel`, `email`, `idUsuarioRel`, `idCadastroRel`, `idProfissionalRel`, `incompleto`) VALUES (4, 'PF', 'CLIENTE', 'Zé', NULL, '927.636.342-49', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 1);

COMMIT;


-- -----------------------------------------------------
-- Data for table `mydb`.`Transportadora`
-- -----------------------------------------------------
START TRANSACTION;
USE `mydb`;
INSERT INTO `mydb`.`Transportadora` (`idTransportadora`, `idEndereco`, `razaoSocial`, `nomeFantasia`, `cnpj`, `inscEstadual`, `placaVeiculo`, `antt`, `tel`) VALUES (1, 1, 'Correios', 'Sedex', '111111111', '111111', NULL, NULL, NULL);

COMMIT;


-- -----------------------------------------------------
-- Data for table `mydb`.`Alcadas`
-- -----------------------------------------------------
START TRANSACTION;
USE `mydb`;
INSERT INTO `mydb`.`Alcadas` (`idAlcada`, `idLoja`, `tipoFuncionario`, `debito`, `credito`, `cheque`, `dinheiro`, `boleto`) VALUES (1, 1, 'Vendedor', 4, 2, 4, 5, 5);
INSERT INTO `mydb`.`Alcadas` (`idAlcada`, `idLoja`, `tipoFuncionario`, `debito`, `credito`, `cheque`, `dinheiro`, `boleto`) VALUES (2, 1, 'Gerente', 8, 4, 8, 10, 10);
INSERT INTO `mydb`.`Alcadas` (`idAlcada`, `idLoja`, `tipoFuncionario`, `debito`, `credito`, `cheque`, `dinheiro`, `boleto`) VALUES (3, 1, 'Administrador', 16, 8, 16, 20, 20);

COMMIT;


-- -----------------------------------------------------
-- Data for table `mydb`.`Endereco`
-- -----------------------------------------------------
START TRANSACTION;
USE `mydb`;
INSERT INTO `mydb`.`Endereco` (`idEndereco`, `descricao`, `cep`, `logradouro`, `numero`, `complemento`, `bairro`, `cidade`, `uf`, `ativo`, `idCadastro`) VALUES (1, 'Não há', '', '', '', '', '', '', '', 1, NULL);
INSERT INTO `mydb`.`Endereco` (`idEndereco`, `descricao`, `cep`, `logradouro`, `numero`, `complemento`, `bairro`, `cidade`, `uf`, `ativo`, `idCadastro`) VALUES (2, 'END. PAGAMENTO', '17521-270', 'Rua Antônio Asperti', '658', 'AP 1', 'Jardim Esplanada', 'Marília', 'SP', 1, 1);
INSERT INTO `mydb`.`Endereco` (`idEndereco`, `descricao`, `cep`, `logradouro`, `numero`, `complemento`, `bairro`, `cidade`, `uf`, `ativo`, `idCadastro`) VALUES (3, 'END. ENTREGA', '54230-586', 'Rua Santa Ana', '14', '', 'Ur-06', 'Jaboatão dos Guararapes', 'PE', 1, 1);
INSERT INTO `mydb`.`Endereco` (`idEndereco`, `descricao`, `cep`, `logradouro`, `numero`, `complemento`, `bairro`, `cidade`, `uf`, `ativo`, `idCadastro`) VALUES (4, 'END. FATURAMENTO', '88813-900', 'Av. Manoel D. Freitas', '1001', '', 'PROSPERA', 'Criciúma', 'SC', 1, 1);
INSERT INTO `mydb`.`Endereco` (`idEndereco`, `descricao`, `cep`, `logradouro`, `numero`, `complemento`, `bairro`, `cidade`, `uf`, `ativo`, `idCadastro`) VALUES (5, 'END. LOJA', '12233-000', 'Av. Andrômeda', '3752', '', 'Bosque dos Eucaliptos', 'São José dos Campos', 'SP', 1, NULL);
INSERT INTO `mydb`.`Endereco` (`idEndereco`, `descricao`, `cep`, `logradouro`, `numero`, `complemento`, `bairro`, `cidade`, `uf`, `ativo`, `idCadastro`) VALUES (6, 'RESIDENCIA', '06543-260', 'Rua Cerejeira', '956', '', 'Condomínio Melville (Tamboré)', 'Santana de Parnaíba', 'SP', 1, 3);
INSERT INTO `mydb`.`Endereco` (`idEndereco`, `descricao`, `cep`, `logradouro`, `numero`, `complemento`, `bairro`, `cidade`, `uf`, `ativo`, `idCadastro`) VALUES (7, 'END. FATURAMENTO', '87114-250 ', 'Avenida da Laguna', '1776', '', 'Jardim Independência III', 'Sarandi', 'PR', 1, 2);

COMMIT;

