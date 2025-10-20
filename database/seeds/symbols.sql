-- Seed data for common stock symbols
-- This file contains initial symbol data for testing and development

-- Technology stocks
INSERT INTO symbols (symbol, name, exchange, sector, industry, market_cap, description) VALUES
('AAPL', 'Apple Inc.', 'NASDAQ', 'Technology', 'Consumer Electronics', 3000000000000, 'Apple Inc. designs, manufactures, and markets smartphones, personal computers, tablets, wearables, and accessories worldwide.'),
('MSFT', 'Microsoft Corporation', 'NASDAQ', 'Technology', 'Software', 2800000000000, 'Microsoft Corporation develops, licenses, and supports software, services, devices, and solutions worldwide.'),
('GOOGL', 'Alphabet Inc.', 'NASDAQ', 'Technology', 'Internet Services', 1700000000000, 'Alphabet Inc. provides online advertising services in the United States, Europe, the Middle East, Africa, the Asia-Pacific, Canada, and Latin America.'),
('AMZN', 'Amazon.com, Inc.', 'NASDAQ', 'Consumer Discretionary', 'E-Commerce', 1600000000000, 'Amazon.com, Inc. engages in the retail sale of consumer products and subscriptions in North America and internationally.'),
('META', 'Meta Platforms, Inc.', 'NASDAQ', 'Technology', 'Internet Services', 900000000000, 'Meta Platforms, Inc. develops products that enable people to connect and share through mobile devices, personal computers, virtual reality headsets, and wearables worldwide.'),
('TSLA', 'Tesla, Inc.', 'NASDAQ', 'Consumer Discretionary', 'Auto Manufacturers', 800000000000, 'Tesla, Inc. designs, develops, manufactures, leases, and sells electric vehicles, and energy generation and storage systems in the United States, China, and internationally.'),
('NVDA', 'NVIDIA Corporation', 'NASDAQ', 'Technology', 'Semiconductors', 1100000000000, 'NVIDIA Corporation provides graphics processing units, system on a chip units, and related software to the gaming and professional markets.');

-- Financial stocks
INSERT INTO symbols (symbol, name, exchange, sector, industry, market_cap, description) VALUES
('JPM', 'JPMorgan Chase & Co.', 'NYSE', 'Financial Services', 'Banks', 500000000000, 'JPMorgan Chase & Co. operates as a financial services company worldwide.'),
('BAC', 'Bank of America Corporation', 'NYSE', 'Financial Services', 'Banks', 250000000000, 'Bank of America Corporation operates as a bank holding company that provides banking and financial products and services for individual consumers, small and middle-market businesses, institutional investors, large corporations, and governments worldwide.'),
('WFC', 'Wells Fargo & Company', 'NYSE', 'Financial Services', 'Banks', 150000000000, 'Wells Fargo & Company provides banking, investment, mortgage, and consumer and commercial finance products and services in the United States and internationally.'),
('GS', 'The Goldman Sachs Group, Inc.', 'NYSE', 'Financial Services', 'Investment Banking', 120000000000, 'The Goldman Sachs Group, Inc. provides a range of financial services for corporations, financial institutions, governments, and individuals worldwide.');

-- Healthcare stocks
INSERT INTO symbols (symbol, name, exchange, sector, industry, market_cap, description) VALUES
('JNJ', 'Johnson & Johnson', 'NYSE', 'Healthcare', 'Drug Manufacturers', 450000000000, 'Johnson & Johnson researches and develops, manufactures, and sells pharmaceutical products and medical devices worldwide.'),
('PFE', 'Pfizer Inc.', 'NYSE', 'Healthcare', 'Drug Manufacturers', 220000000000, 'Pfizer Inc. discovers, develops, manufactures, and sells biopharmaceutical products worldwide.'),
('UNH', 'UnitedHealth Group Incorporated', 'NYSE', 'Healthcare', 'Healthcare Plans', 500000000000, 'UnitedHealth Group Incorporated provides healthcare services and products in the United States.');

-- Industrial stocks
INSERT INTO symbols (symbol, name, exchange, sector, industry, market_cap, description) VALUES
('BA', 'The Boeing Company', 'NYSE', 'Industrials', 'Aerospace & Defense', 120000000000, 'The Boeing Company, together with its subsidiaries, designs, develops, manufactures, sales, and services commercial jetliners, military aircraft, satellites, missile defense, human space flight and launch systems, and services worldwide.'),
('CAT', 'Caterpillar Inc.', 'NYSE', 'Industrials', 'Construction Machinery', 100000000000, 'Caterpillar Inc. manufactures and sells construction and mining equipment, diesel and natural gas engines, industrial gas turbines, and diesel-electric locomotives worldwide.'),
('GE', 'General Electric Company', 'NYSE', 'Industrials', 'Conglomerates', 90000000000, 'General Electric Company operates as a digital industrial company worldwide.');

-- Consumer stocks
INSERT INTO symbols (symbol, name, exchange, sector, industry, market_cap, description) VALUES
('WMT', 'Walmart Inc.', 'NYSE', 'Consumer Staples', 'Discount Stores', 400000000000, 'Walmart Inc. engages in the operation of retail, wholesale, and other units worldwide.'),
('PG', 'The Procter & Gamble Company', 'NYSE', 'Consumer Staples', 'Household & Personal Products', 350000000000, 'The Procter & Gamble Company provides branded consumer packaged goods worldwide.'),
('KO', 'The Coca-Cola Company', 'NYSE', 'Consumer Staples', 'Beverages', 270000000000, 'The Coca-Cola Company manufactures, markets, and sells various beverages worldwide.'),
('MCD', 'McDonald''s Corporation', 'NYSE', 'Consumer Discretionary', 'Restaurants', 200000000000, 'McDonald''s Corporation operates and franchises McDonald''s restaurants in the United States and internationally.');

-- Energy stocks
INSERT INTO symbols (symbol, name, exchange, sector, industry, market_cap, description) VALUES
('XOM', 'Exxon Mobil Corporation', 'NYSE', 'Energy', 'Oil & Gas Integrated', 400000000000, 'Exxon Mobil Corporation explores for and produces crude oil and natural gas in the United States and internationally.'),
('CVX', 'Chevron Corporation', 'NYSE', 'Energy', 'Oil & Gas Integrated', 300000000000, 'Chevron Corporation operates through its Upstream and Downstream segments worldwide.');

-- Index ETFs
INSERT INTO symbols (symbol, name, exchange, sector, industry, market_cap, description) VALUES
('SPY', 'SPDR S&P 500 ETF Trust', 'AMEX', 'ETF', 'Large Cap Blend', 450000000000, 'The SPDR S&P 500 ETF Trust is an exchange-traded fund that tracks the Standard & Poor''s 500 index.'),
('QQQ', 'Invesco QQQ Trust', 'NASDAQ', 'ETF', 'Large Cap Growth', 200000000000, 'Invesco QQQ Trust is an exchange-traded fund that tracks the NASDAQ-100 index.'),
('VTI', 'Vanguard Total Stock Market ETF', 'AMEX', 'ETF', 'Large Cap Blend', 300000000000, 'Vanguard Total Stock Market ETF is an exchange-traded fund that tracks the CRSP US Total Market Index.');