 figure(1);clf;
 figure(3);clf;
%   figure(2);clf;

N = 400; %Noumber of frames or it chould be frames*bands
s=[1;30];  
% true location of sound in polar coordinate system ( r, theta). For now r is always 1 and theta is variable 
% theta varies from -90 to 90 and zero is at the centre.

% In this version mics array configuration is fixed relative to the global coordinate system ( it is not rotating)
%collecting N frames of audio, and for each frame, the beamformer localize the sound at
%some location. The error in estimation of where the sound is will be
%defined as beam pattern probability density function(pdf) around where the sound is actually located 

%% test recursive Bayesian regardless of beamformer by uncommenting this section
%just for test instead of going to the beamformer localizer and collecting data

% n1=20*(rand(1,N)-.5); % Create vector of iterative localized sound with a standard deviation of 2.
var=5;% variance of the data
n1=var*(randn(1,N)); 

n=zeros(2,N);
n(2,:)=n1;
%x=zeros(2,N); % x will be the initialized variable for where the robot thinks hears the sound.

%center the Gaussian random sound around the point where the sound is really located and plot
figure(1);

%make the plot prettier
h=plot(s(1),s(2),'r.');  % Plot where the sound actually is
set(h,'markersize',40,'linewidth',3); % make pretty
axis([0,2,0,180]);  % make pretty
hold off;
hold on
for i=1:N
    x(:,i)=s+n(:,i);
    if x(2,i)<=1
       x(2,i)=1;
    end
    
    if x(2,i)>179
       x(2,i)=179;
    end
    plot(x(1,i),x(2,i),'k.','markersize',10);
       
end


%now, we do recursive Bayesian

%define the range of locations the sound can be at
% Sa=[.6:0.2:2]; % r
Sa=1;
Sb=[0:1:180]; % theta


%no bias, no prior knowledge, uniform distribution
nn=length(Sa);
mm=length(Sb);
Pr=ones(nn,mm); % Initialize all one --> uniform prior
Po=ones(nn,mm); %duplicate for iterative update

%bias the prior toward the true location sound
%figure(1);mesh(centered_prior), axis([0 40 0 40 0 0.015])
%Po = centered_prior;
%Pr = centered_prior;

%bias the prior away from the true location sound
%figure(1);mesh(off_centered_prior), axis([0 40 0 40 0 0.015])
%Po = off_centered_prior;
%Pr = off_centered_prior;

Pr=Pr/sum(sum(Pr)); % Turn the prior into a pdf by dividing by the sum.
Po=Po/sum(sum(Po)); % Each value is now 1/(number of states), so it all sums to one.
% figure(1);clf;mesh(Po), axis([.6 2 -90 90 0 0.015])
figure(3);clf;plot(Po), axis([0 180 0 1])


%%iterative bayes

[a,b]=find(Po==max(max(Po)));  % Pull out the indices at which Po achieves its max to start.
sest=[Sa(a);Sb(b)];  % The best estimate of the true state to start.



% figure(2);
% clf
% subplot(211); plot(1,sest(1)); hold on;axis([0 100 0 2])
% line([1,N],[s(1),s(1)]); % Draw a line at the location of the x dimension.
% subplot(212); plot(1,sest(2)); hold on;axis([0 100 0 180])
% line([1,N],[s(2),s(2)]); % Draw a line at the location of the y dimension.

%instead of the beampattern the gaussian pdf as an approximation can be used. 
%K=[4,0;0,4]; % covariance matrix for making a 2-D gaussian
for k=2:length(x);
    for ff=1:15 % number of band 
%         ff
    Pr=Po; %store the posterior to the prior.
    m=0*Pr;   
    %likelihood
    % look at each location, assume that the given location is
    % is there sound is, and get the likelihood of the data x(:,n)
    % 1- assuming 2-d gaussian noise 2- beamformer pattern liklihood
    
    
    for i=1:nn
       for j=1:mm-1
           me=[Sa(i);Sb(j)];
        %   m(i,j) = 1/sqrt((2*pi)^2*det(K)) * exp(-(x(:,n)-me)'*inv(K)*(x(:,n)-me)/2); %Compute likelihood           
		
		%beam pattern pdf here
%         pdfv2(frn,SteeringAngle+1,:)
        if x(2,k)<90
        m(i,j)=   pdfv2(ff, round(x(2,k)),round((500-1)*(me(2,1) )/180+1));%Sb(j)+1
        else
		m(i,j)=   pdfv2(ff,181-round(x(2,k)),round((500-1)*(me(2,1))/180+1));
        end
        m(i,j) = m(i,j) * Pr(i,j); % Combine this likelihood with the prior   
        0;
       end
    end
    Po=m/sum(sum(m)); %normalize this distribution to make it a proper probability distribution.
%     figure(1);mesh(Po), axis([.6 2 1 180 0 0.1]) %plot it
    figure(3);plot(Po), axis([1 180 0 1]) %plot it
    
    [a,b]=find(Po==max(max(Po)));  % Get the peak value; it's most likely location of the sound.
    sest=[Sa(a);Sb(b)];  %store the coordinates of this location
    
    
%     figure(2);
%     subplot(211); plot(k,sest(1),'k.');axis([0 N .6 2 ])
%     subplot(212); plot(k,sest(2),'k.');axis([0 N 0 180 ])
    pause(.01)
    end
end 
% subplot(211); hold off;
% subplot(212); hold off;