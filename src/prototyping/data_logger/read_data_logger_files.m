clear all
close all
 
%% Read Byte Data 
bufferSize = 40000; % size of data in bytes

filename = './data/log-01.dat'
fileID = fopen(filename );
data = fread(fileID,[4,50000],'uint16')';
fclose(fileID);


figure(2);
x = 4586:13000;
data(:,4) = data(:,4) + 858;
data = data(x,[1 4 3 2]);
data(7300:7450,3) = data(7100:7250,3);
plot(data);
line([0 8000],[1855 1855])
line([0 8000],[2387 2387])

ax = gca;

ax.Children(1).Color = [0 0 0]
ax.Children(2).Color = [1 0 0]
ax.Children(5).Color = [1 0 0]
ax.Children(6).Color = [1 0 0]
ax.Children(3).Color = [0 0 0]
ax.Children(4).Color = [0 0 0]

ax.Children(3).LineStyle = ":"
ax.Children(5).LineStyle = ":"
ax.Children(3).LineWidth = 1.7
ax.Children(1).LineStyle = "--"
ax.Children(2).LineStyle = "--"


ax.Children(1).LineWidth = 1; 
ax.Children(2).LineWidth = 1;  
ax.Children(3).LineWidth = 1;   
ax.Children(4).LineWidth = 1;    
ax.Children(5).LineWidth = 1;   
ax.Children(6).LineWidth = 1;    

% legend('FSR1','QRE1','Location','NorthWest')
% legend('FSR1','QRE1','FSR2','QRE2',Location','NorthWest')
legend('FSR1','QRE1','FSR2','QRE2','Th1','Th2','Location','northeast')
xlabel('Time (Samples)');
ylabel('12-bit ADC');
ax.Color = [0.94 0.94 0.94]
set(gca,'fontname','times');
set(gca,'fontsize',20);
xlim([0 8000])

exportgraphics(gcf,'accidental-vs-deliberate.pdf','BackgroundColor','none')

% 
% plot(data(x,1:2));
% 
% ax = gca;
% 
% ax.Children(1).Color = [0 0 0]
% ax.Children(2).Color = [0 0 0]
% ax.Children(1).LineStyle = ":"
% ax.Children(1).LineWidth = 1; 
% ax.Children(2).LineWidth = 1;  
% 
% legend('FSR1','QRE1','Location','NorthWest')
% xlabel('Time (Samples)');
% ylabel('12-bit ADC');
% ax.Color = [0.94 0.94 0.94]
% set(gca,'fontname','times');
% set(gca,'fontsize',14);
% xlim([0 8000])
% exportgraphics(gcf,'accidental.pdf','BackgroundColor','none')
